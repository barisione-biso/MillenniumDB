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

    // std::cout << "Size bytes:     " << sizeof(bytes) << "\n";
    // std::cout << "Size &bytes:    " << sizeof(&bytes) << "\n";
    // std::cout << "Size bytes[0]:  " << sizeof(bytes[0]) << "\n";
    // std::cout << "Size &bytes[0]: " << sizeof(&bytes[0]) << "\n";
}

BufferManager::~BufferManager()
{
}

void BufferManager::flush()
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
    // cout << "append_page(" << filename << ")\n";
    return get_page(count_pages(filename) ,filename);
}

int BufferManager::get_buffer_available()
{
    int first_lookup = clock_pos;
    while (buffer_pool[clock_pos] != nullptr && buffer_pool[clock_pos]->pins != 0) {
        clock_pos = (clock_pos+1)%BUFFER_POOL_INITIAL_SIZE;
        if (clock_pos == first_lookup) {
            throw std::logic_error("No buffer available.");
        }
    }
    int res = clock_pos;
    clock_pos = (clock_pos+1)%BUFFER_POOL_INITIAL_SIZE;
    return res;
}

Page& BufferManager::get_page(int page_number, const string& filename) {
    // cout << "get_page(" << page_number << ", " << filename << ")\n";
    if (page_number != 0 && count_pages(filename) < page_number) {
        std::cout << "getting wrong page_number for " << filename << " (" << page_number << "), max: " << count_pages(filename) << ".\n";

        throw std::logic_error("getting wrong page_number.");
    }
    pair<string, int> page_key = pair<string, int>(filename, page_number);
    map<pair<string, int>, int>::iterator it = pages.find(page_key);

    if (it == pages.end()) {
        int buffer_available = get_buffer_available();
        if (buffer_pool[buffer_available] == nullptr) {
            buffer_pool[buffer_available] = new Page(page_number, &bytes[buffer_available*PAGE_SIZE], filename);
        }
        else {
            // std::cout << "REUSING PAGE\n";
            pair<string, int> old_page_key = pair<string, int>(buffer_pool[buffer_available]->filename, buffer_pool[buffer_available]->page_number);
            pages.erase(old_page_key);
            // buffer_pool[buffer_available]->reuse(page_number, filename);
            delete buffer_pool[buffer_available];
            buffer_pool[buffer_available] = new Page(page_number, &bytes[buffer_available*PAGE_SIZE], filename);
        }

        fstream file(filename, fstream::in|fstream::out|fstream::binary|fstream::app);
        file.seekg (0, file.end);
        if (file.tellg()/PAGE_SIZE <= page_number) { // new file block
            for (int i = 0; i < PAGE_SIZE; i++) {
                buffer_pool[buffer_available]->get_bytes()[i] = 0;
            }
            file.write(buffer_pool[buffer_available]->get_bytes(), PAGE_SIZE);
        }
        else { // existing file block
            file.seekg(page_number*PAGE_SIZE);
            file.read(buffer_pool[buffer_available]->get_bytes(), PAGE_SIZE);
        }
        file.close();

        pages.insert(pair<pair<string, int>, int>(page_key, buffer_available));
        return *buffer_pool[buffer_available];
    }
    else { // file is already open
        buffer_pool[it->second]->pin();
        return *buffer_pool[it->second];
    }
}