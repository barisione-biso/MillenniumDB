#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>

#include "file/page.h"
#include "file/buffer_manager.h"

#define BUFFER_POOL_SIZE 4096

using namespace std;

BufferManager::BufferManager()
{
    buffer_pool = new Buffer*[BUFFER_POOL_SIZE];
    pages_pool = new Page*[BUFFER_POOL_SIZE];
    clock_pos = 0;
}

BufferManager::~BufferManager()
{
    cout << "FLUSHING PAGES\n";
    for (int i = 0; i < PAGE_SIZE; i++) {
        if (pages_pool[i] != nullptr) {
            pages_pool[i]->flush();
        }
    }
}

int BufferManager::count_pages(const string& filename)
{
    return boost::filesystem::file_size(filename)/PAGE_SIZE;
}

Page& BufferManager::append_page(const string& filename)
{
    return get_page(count_pages(filename) ,filename);
}

int BufferManager::get_buffer_available()
{
    return clock_pos++; // TODO:
}

Page& BufferManager::get_page(int page_number, const string& filename) {
    pair<string, int> page_key = pair<string, int>(filename, page_number);
    map<pair<string, int>, int>::iterator it = pages.find(page_key);

    if (it == pages.end()) {
        int buffer_available = get_buffer_available();
        if (buffer_pool[buffer_available] == nullptr) {
            buffer_pool[buffer_available] = new Buffer();
        }
        fstream file;
        file.open(filename, fstream::in|fstream::out|fstream::binary|fstream::app);

        // check file has block_size == page_number+1?
        // cout << "get_page(" <<page_number << ", "<< filename << ")\n";
        file.seekg (0, file.end);
        int block_size = file.tellg()/PAGE_SIZE;
        // cout << "  block_size: " << block_size << "\n";

        if (block_size <= page_number) {
            for (int i = 0; i < PAGE_SIZE; i++) {
                buffer_pool[buffer_available]->get_bytes()[i] = 0;
            }
            file.write(buffer_pool[buffer_available]->get_bytes(), PAGE_SIZE);
            // cout << "  New block created\n";
        }
        else {
            // cout << "  buffer_available" << buffer_available << "\n";
            file.seekg(page_number*PAGE_SIZE);
            file.readsome(buffer_pool[buffer_available]->get_bytes(), PAGE_SIZE);
        }

        file.close();

        pages.insert(std::pair<pair<string, int>, int>(page_key, buffer_available));
        pages_pool[buffer_available] = new Page(page_number, *buffer_pool[buffer_available], filename);
        // cout << "returned get_page(" <<page_number << ", "<< filename << ")\n";
        return *pages_pool[buffer_available];
    }
    else { // file is already open
        pages_pool[it->second]->pin();
        return *pages_pool[it->second];
    }
}