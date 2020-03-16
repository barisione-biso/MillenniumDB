// !!!! IMPORTANT !!!! NEVER include this file in a header file

#ifndef STORAGE__BUFFER_MANAGER_H_
#define STORAGE__BUFFER_MANAGER_H_

#include "storage/file_id.h"

#include <map>
#include <string>

class Page;
class Buffer;

struct PageId {
    FileId file_id;
    uint_fast32_t page_number;

    PageId(FileId file_id, uint_fast32_t page_number)
        : file_id(file_id), page_number(page_number) { }

    bool operator<(const PageId& other) const {
        if (this->file_id < other.file_id) {
            return true;
        } else if (other.file_id < this->file_id) {
            return false;
        } else {
            return this->page_number < other.page_number;
        }
    }
};


class BufferManager {
friend class BufferManagerInitializer;
public:
    Page& get_page(uint_fast32_t page_number, FileId file_id);
    Page& append_page(FileId file_id);
    /* all dirty pages are written to disk */
    void flush();

private:
    std::map<PageId, int> pages;

    Page* buffer_pool;
    char* bytes;
    int clock_pos;

    BufferManager();
    ~BufferManager();
    int get_buffer_available();
};

extern BufferManager& buffer_manager; // global object

static struct BufferManagerInitializer {
    BufferManagerInitializer();
    ~BufferManagerInitializer();
} bufferManager_initializer; // static initializer for every translation unit

#endif // STORAGE__BUFFER_MANAGER_H_
