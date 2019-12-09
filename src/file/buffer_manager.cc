#include <iostream>

#include "file/page.h"
#include "file/buffer_manager.h"
#include "file/file_manager.h"

#define BUFFER_POOL_INITIAL_SIZE 4096

using namespace std;

BufferManager BufferManager::instance = BufferManager();

BufferManager::BufferManager()
{
    buffer_pool = new Page[BUFFER_POOL_INITIAL_SIZE];
    clock_pos = 0;
    bytes = new char[BUFFER_POOL_INITIAL_SIZE*PAGE_SIZE];

    // for (int i = 0; i < BUFFER_POOL_INITIAL_SIZE; i++) {
    //     buffer_pool[i] = nullptr;
    // }
}

BufferManager::~BufferManager()
{
    std::cout << "~BufferManager()\n";
    if (!instance.flushed_at_exit) {
        _flush();
        instance.flushed_at_exit = true;
        FileManager::instance.flushed_at_exit = true;
    }
}

Page& BufferManager::get_page(uint_fast32_t page_number, FileId file_id)
{
    return instance._get_page(page_number, file_id);
}

Page& BufferManager::append_page(FileId file_id)
{
    return instance._append_page(file_id);
}

void BufferManager::_flush()
{
    // cout << "FLUSHING PAGES\n";
    for (int i = 0; i < PAGE_SIZE; i++) {
        buffer_pool[i].flush();
    }
}

Page& BufferManager::_append_page(FileId file_id)
{
    // cout << "append_page(" << file_path << ")\n";
    return get_page(FileManager::count_pages(file_id), file_id);
}

int BufferManager::get_buffer_available()
{
    int first_lookup = clock_pos;
    while (buffer_pool[clock_pos].pins != 0) {
        clock_pos = (clock_pos+1)%BUFFER_POOL_INITIAL_SIZE;
        if (clock_pos == first_lookup) {
            throw std::logic_error("No buffer available.");
        }
    }
    int res = clock_pos;
    clock_pos = (clock_pos+1)%BUFFER_POOL_INITIAL_SIZE;
    return res;
}

Page& BufferManager::_get_page(uint_fast32_t page_number, FileId file_id) {
    // cout << "get_page(" << page_number << ", " << file_path << ")\n";
    if (page_number != 0 && FileManager::count_pages(file_id) < page_number) {
        std::cout << "Page Number: " << page_number << ", FileId: " << file_id.id << "(" << FileManager::get_filename(file_id) <<  ")\n";
        throw std::logic_error("getting wrong page_number.");
    }
    pair<FileId, int> page_key = pair<FileId, int>(file_id, page_number);
    map<pair<FileId, int>, int>::iterator it = pages.find(page_key);

    if (it == pages.end()) {
        int buffer_available = get_buffer_available();
        if (buffer_pool[buffer_available].file_id.id != FileId::UNASSIGNED) {
            pair<FileId, int> old_page_key = pair<FileId, int>(buffer_pool[buffer_available].file_id,
                                                               buffer_pool[buffer_available].page_number);
            pages.erase(old_page_key);
        }
        buffer_pool[buffer_available] = Page(page_number, &bytes[buffer_available*PAGE_SIZE], file_id);

        FileManager::read_page(page_key.first, page_number, buffer_pool[buffer_available].get_bytes());
        pages.insert(pair<pair<FileId, int>, int>(page_key, buffer_available));
        return buffer_pool[buffer_available];
    }
    else { // file is already open
        buffer_pool[it->second].pin();
        return buffer_pool[it->second];
    }
}