#include "file_manager.h"

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <cassert>
#include <cstring>
#include <new>         // placement new
#include <type_traits> // aligned_storage

#include "storage/buffer_manager.h"
#include "storage/file_id.h"
#include "storage/page.h"

using namespace std;

// memory for the object
static typename std::aligned_storage<sizeof(FileManager), alignof(FileManager)>::type file_manager_buf;
// global object
FileManager& file_manager = reinterpret_cast<FileManager&>(file_manager_buf);


FileManager::FileManager(const std::string& db_folder) :
    db_folder(db_folder)
{
    if (Filesystem::exists(db_folder)) {
        if (!Filesystem::is_directory(db_folder)) {
            throw std::invalid_argument("Cannot create database directory: \"" + db_folder +
                                        "\", a file with that name already exists.");
        }
    } else {
        Filesystem::create_directories(db_folder);
    }
}


void FileManager::init(const std::string& db_folder) {
    new (&file_manager) FileManager(db_folder); // placement new
}


void FileManager::flush(PageId page_id, char* bytes) const {
    auto fd = opened_files[page_id.file_id.id];
    lseek(fd, page_id.page_number*Page::MDB_PAGE_SIZE, SEEK_SET);
    auto write_res = write(fd, bytes, Page::MDB_PAGE_SIZE);
    if (write_res == -1) {
        throw std::runtime_error("Could not write into file when flushing page");
    }
}


void FileManager::read_page(PageId page_id, char* bytes) const {
    auto fd = opened_files[page_id.file_id.id];
    lseek(fd, 0, SEEK_END);

    struct stat buf;
    fstat(fd, &buf);
    uint64_t file_size = buf.st_size;

    lseek(fd, page_id.page_number*Page::MDB_PAGE_SIZE, SEEK_SET);
    if (file_size/Page::MDB_PAGE_SIZE <= page_id.page_number) {
        // new file page, write zeros
        memset(bytes, 0, Page::MDB_PAGE_SIZE);
        auto write_res = write(fd, bytes, Page::MDB_PAGE_SIZE);
        if (write_res == -1) {
            throw std::runtime_error("Could not write into file");
        }
    } else {
        // reading existing file page
        auto read_res = read(fd, bytes, Page::MDB_PAGE_SIZE);
        if (read_res == -1) {
            throw std::runtime_error("Could not read file page");
        }
    }
}


FileId FileManager::get_file_id(const string& filename) {
    std::lock_guard<std::mutex> lck(files_mutex);

    // case 1: file is in the map
    auto search = filename2file_id.find(filename);
    if (search != filename2file_id.end()) {
        return search->second;
    }

    // case 2: file is not in the map and available_file_ids is not empty
    else if (!available_file_ids.empty()) {
        const auto file_path = get_file_path(filename);
        const auto res = available_file_ids.front();
        available_file_ids.pop();

        filenames[res.id] = filename;
        filename2file_id.insert({ filename, res });

        auto fd = open(file_path.c_str(), O_RDWR/*|O_DIRECT*/|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
        if (fd == -1) {
            throw std::runtime_error("Could not open file " + file_path);
        }
        opened_files[res.id] = fd;
        return res;
    }

    // case 3: file is not in the map and available_file_ids is empty
    else {
        const auto file_path = get_file_path(filename);
        const auto res = FileId(filenames.size());

        filenames.push_back(filename);
        filename2file_id.insert({ filename, res });

        auto fd = open(file_path.c_str(), O_RDWR/*|O_DIRECT*/|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
        if (fd == -1) {
            throw std::runtime_error("Could not open file " + file_path);
        }

        opened_files.push_back(fd);
        return res;
    }
}


TmpFileId FileManager::get_tmp_file_id() {
    std::lock_guard<std::mutex> lck(files_mutex);

    string filename = "tmp" + std::to_string(tmp_filename_counter++);

    if (!available_file_ids.empty()) {
        const auto file_path = get_file_path(filename);
        const auto file_id = available_file_ids.front();
        available_file_ids.pop();

        filenames[file_id.id] = filename;
        filename2file_id.insert({ filename, file_id });
        auto fd = open(file_path.c_str(), O_RDWR/*|O_DIRECT*/|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
        if (fd == -1) {
            throw std::runtime_error("Could not open file " + file_path);
        }
        opened_files[file_id.id] = fd;
        return TmpFileId(buffer_manager.get_private_buffer_index(), file_id);
    }
    else {
        const auto file_path = get_file_path(filename);
        const auto file_id = FileId(filenames.size());

        filenames.push_back(filename);
        filename2file_id.insert({ filename, file_id });
        auto fd = open(file_path.c_str(), O_RDWR/*|O_DIRECT*/|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
        if (fd == -1) {
            throw std::runtime_error("Could not open file " + file_path);
        }
        opened_files.push_back(fd);
        return TmpFileId(buffer_manager.get_private_buffer_index(), file_id);
    }
}


void FileManager::remove(const FileId file_id) {
    std::lock_guard<std::mutex> lck(files_mutex);
    const auto file_path = get_file_path(filenames[file_id.id]);

    buffer_manager.remove(file_id);                 // clear pages from buffer_manager
    close(opened_files[file_id.id]);                // close the file stream
    std::remove(file_path.c_str());                 // delete file from disk

    filename2file_id.erase(filenames[file_id.id]);  // update map
    opened_files[file_id.id] = -1;                  // clean file description
    available_file_ids.push(file_id);               // add removed file_id as available for reuse
}


void FileManager::remove_tmp(const TmpFileId tmp_file_id) {
    std::lock_guard<std::mutex> lck(files_mutex);
    const auto file_path = get_file_path(filenames[tmp_file_id.file_id.id]);

    buffer_manager.remove_tmp(tmp_file_id);                     // clear pages from buffer_manager
    close(opened_files[tmp_file_id.file_id.id]);                // close the file stream
    std::remove(file_path.c_str());                             // delete file from disk

    filename2file_id.erase(filenames[tmp_file_id.file_id.id]);  // update map
    opened_files[tmp_file_id.file_id.id] = -1;                  // clean file description
    available_file_ids.push(tmp_file_id.file_id);               // add removed file_id as available for reuse
}
