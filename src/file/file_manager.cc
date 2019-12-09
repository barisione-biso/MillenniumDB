#include "file/file_manager.h"

#include "file/buffer_manager.h"

#include <algorithm>
#include <cstdio>
#include <experimental/filesystem>
#include "file/page.h"

using namespace std;

FileManager FileManager::instance = FileManager();

FileManager::FileManager() {

}

FileManager::~FileManager() {
    std::cout << "~FileManager()\n";
    if (!flushed_at_exit) {
        BufferManager::instance._flush();
        BufferManager::instance.flushed_at_exit = true;
    }
    // TODO: close files still open from opened_files
}

void FileManager::ensure_open(FileId file_id) {
    if (!instance.opened_files[file_id.id]->is_open()) {
        if (!experimental::filesystem::exists(instance.filenames[file_id.id])) {
            instance.opened_files[file_id.id]->open(instance.filenames[file_id.id], ios::out|ios::app);
            instance.opened_files[file_id.id]->close();
        }
        // std::cout  << "re opening " << instance.filenames[file_id.id] << " \n";
        instance.opened_files[file_id.id]->open(instance.filenames[file_id.id], ios::in|ios::out|ios::binary);
    }
}

void FileManager::close(FileId file_id) {
    instance.opened_files[file_id.id]->close();
}

void FileManager::remove(FileId file_id) {
    close(file_id);
    std::remove(instance.filenames[file_id.id].c_str());
}

void FileManager::rename(FileId old_name_id, FileId new_name_id) {
    close(old_name_id);
    close(new_name_id);

    experimental::filesystem::rename(instance.filenames[old_name_id.id], instance.filenames[new_name_id.id]);
}

uint_fast32_t FileManager::count_pages(FileId file_id)
{
    return experimental::filesystem::file_size(instance.filenames[file_id.id])/PAGE_SIZE;
}

string FileManager::get_filename(FileId file_id) {
    return instance.filenames[file_id.id];
}


void FileManager::flush(FileId file_id, uint_fast32_t page_number, char* bytes)
{
    fstream& file = get_file(file_id);
    file.seekp(page_number*PAGE_SIZE);
    file.write(bytes, PAGE_SIZE);
}


void FileManager::read_page(FileId file_id, uint_fast32_t page_number, char* bytes)
{
    fstream& file = get_file(file_id);
    file.seekg(0, file.end);
    uint_fast32_t file_pos = file.tellg();
    if (file_pos/PAGE_SIZE <= page_number) {   // new file block
        for (int i = 0; i < PAGE_SIZE; i++) {
            bytes[i] = 0;
        }
        file.write(bytes, PAGE_SIZE);
    }
    else {                                     // existing file block
        file.seekg(page_number*PAGE_SIZE);
        file.read(bytes, PAGE_SIZE);
    }
}


fstream& FileManager::get_file(FileId file_id)
{
    ensure_open(file_id);
    return *instance.opened_files[file_id.id];
}

FileId FileManager::get_file_id(const string& filename)
{
    return instance._get_file_id(filename);
}

FileId FileManager::_get_file_id(const string& filename) {
    string file_path = "test_files/" + filename;
    for (size_t i = 0; i < filenames.size(); i++) {
        if (file_path.compare(filenames[i]) == 0) {
            // cout << "duplicated filename\n";
            return FileId(i);
        }
    }

    filenames.push_back(file_path);
    fstream* file = new fstream();
    if (!experimental::filesystem::exists(file_path)) {
        file->open(file_path, ios::out|ios::app);
        file->close();
    }
    file->open(file_path, ios::in|ios::out|ios::binary);
    opened_files.push_back(file);

    return FileId(filenames.size()-1);
}
