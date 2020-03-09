// !!!! IMPORTANT !!!! NEVER include this file in a header file

#ifndef FILE__BUFFER_MANAGER_H_
#define FILE__BUFFER_MANAGER_H_

#include "file/file_id.h"

#include <map>
#include <string>

class Page;
class Buffer;

class BufferManager {
public:
    Page& get_page(uint_fast32_t page_number, FileId file_id);
    Page& append_page(FileId file_id);
    void flush();

    BufferManager(); // TODO: try making private and friend class
    ~BufferManager();

private:
    std::map<std::pair<FileId, int>, int> pages;

    Page* buffer_pool;
    char* bytes;
    int clock_pos;

    int get_buffer_available();
};

extern BufferManager& buffer_manager; // global object

static struct BufferManagerInitializer {
  BufferManagerInitializer();
  ~BufferManagerInitializer();
} bufferManager_initializer; // static initializer for every translation unit


#endif //FILE__BUFFER_MANAGER_H_
