#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>

#include "file/page.h"
#include "file/buffer_manager.h"

#define BUFFER_POOL_INITIAL_SIZE 4096

using namespace std;

BufferManager::BufferManager()
{
    buffer_pool = new Page*[BUFFER_POOL_INITIAL_SIZE];
    clock_pos = 0;
    bytes = new char[BUFFER_POOL_INITIAL_SIZE*PAGE_SIZE];
}

BufferManager::~BufferManager()
{
    cout << "FLUSHING PAGES\n";
    for (int i = 0; i < PAGE_SIZE; i++) {
        if (buffer_pool[i] != nullptr) {
            buffer_pool[i]->flush();
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
            buffer_pool[buffer_available] = new Page(page_number, &bytes[page_number*PAGE_SIZE], filename);
        }
        else {
            buffer_pool[buffer_available]->reuse(page_number, filename);
        }

        fstream file(filename, fstream::in|fstream::out|fstream::binary|fstream::app);
        file.seekg (0, file.end);
        if (file.tellg()/PAGE_SIZE <= page_number) {
            for (int i = 0; i < PAGE_SIZE; i++) {
                buffer_pool[buffer_available]->get_bytes()[i] = 0;
            }
            file.write(buffer_pool[buffer_available]->get_bytes(), PAGE_SIZE);
        }
        else {
            file.seekg(page_number*PAGE_SIZE);
            file.readsome(buffer_pool[buffer_available]->get_bytes(), PAGE_SIZE);
        }
        file.close();

        pages.insert(std::pair<pair<string, int>, int>(page_key, buffer_available));
        return *buffer_pool[buffer_available];
    }
    else { // file is already open
        buffer_pool[it->second]->pin();
        return *buffer_pool[it->second];
    }
}