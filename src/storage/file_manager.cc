#include "file_manager.h"

#include <cassert>
#include <cstring>
#include <experimental/filesystem>
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
    if (experimental::filesystem::exists(db_folder)) {
        if (!experimental::filesystem::is_directory(db_folder)) {
            throw std::runtime_error("Cannot create database directory: \"" + db_folder +
                                     "\", a file with that name already exists.");
        }
    } else {
        experimental::filesystem::create_directories(db_folder);
    }
}


void FileManager::init(const std::string& db_folder) {
    new (&file_manager) FileManager(db_folder); // placement new
}


uint_fast32_t FileManager::count_pages(const FileId file_id) const {
    return experimental::filesystem::file_size(file_paths[file_id.id])/Page::PAGE_SIZE;
}


void FileManager::flush(const PageId page_id, char* bytes) const {
    fstream& file = get_file(page_id.file_id);
    file.seekg(page_id.page_number*Page::PAGE_SIZE);
    file.write(bytes, Page::PAGE_SIZE);
}


void FileManager::read_page(const PageId page_id, char* bytes) const {
    fstream& file = get_file(page_id.file_id);
    file.seekg(0, file.end);
    uint_fast32_t file_pos = file.tellg();
    if (file_pos/Page::PAGE_SIZE <= page_id.page_number) {
        // new file block
        memset(bytes, 0, Page::PAGE_SIZE);
        file.write(bytes, Page::PAGE_SIZE);
    } else {
        // reading existing file block
        file.seekg(page_id.page_number*Page::PAGE_SIZE);
        file.read(bytes, Page::PAGE_SIZE);
    }
}


fstream& FileManager::get_file(const FileId file_id) const {
    assert(file_id.id < opened_files.size());
    assert(opened_files[file_id.id] != nullptr);
    assert(opened_files[file_id.id]->is_open());

    return *opened_files[file_id.id];
}


FileId FileManager::get_file_id(const string& filename) {
    const string file_path = db_folder + "/" + filename;

    std::lock_guard<std::mutex> lck(files_mutex);

    // case 1: file is in the map
    auto search = filename2file_id.find(filename);
    if (search != filename2file_id.end()) {
        return search->second;
    }

    // case 2: file is not in the map and available_file_ids is not empty
    else if (!available_file_ids.empty()) {
        const auto res = available_file_ids.front();
        available_file_ids.pop();

        file_paths[res.id] = file_path;
        filename2file_id.insert({ filename, res });
        auto file = make_unique<fstream>();
        if (!experimental::filesystem::exists(file_path)) {
            // `ios::app` creates the file if it doesn't exists but we don't want it open in append mode,
            // so we close it and open it again without append mode
            file->open(file_path, ios::out|ios::app);
            if (file->fail()) {
                throw std::runtime_error("Could not open file " + file_path);
            }
            file->close();
        }
        file->open(file_path, ios::in|ios::out|ios::binary);
        opened_files[res.id] = move(file);
        return res;
    }

    // case 3: file is not in the map and available_file_ids is empty
    else {
        const auto res = FileId(file_paths.size());

        file_paths.push_back(file_path);
        filename2file_id.insert({ filename, res });
        auto file = make_unique<fstream>();
        if (!experimental::filesystem::exists(file_path)) {
            // `ios::app` creates the file if it doesn't exists but we don't want it open in append mode,
            // so we close it and open it again without append mode
            file->open(file_path, ios::out|ios::app);
            if (file->fail()) {
                throw std::runtime_error("Could not open file " + file_path);
            }
            file->close();
        }
        file->open(file_path, ios::in|ios::out|ios::binary);
        opened_files.push_back(move(file));
        return res;
    }
}


void FileManager::remove(const FileId file_id) {
    std::lock_guard<std::mutex> lck(files_mutex);

    buffer_manager.remove(file_id);                 // clear pages from buffer_manager
    opened_files[file_id.id]->close();              // close the file stream
    std::remove(file_paths[file_id.id].c_str());    // delete file from disk

    filename2file_id.erase(file_paths[file_id.id]); // update map
    // file_paths[file_id.id] = "";                    // update file_paths, maybe is redundant?
    opened_files[file_id.id].reset();               // destroy the fstream
    available_file_ids.push(file_id);               // add removed file_id as available for reuse
}
