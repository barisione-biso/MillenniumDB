#ifndef FILE__BUFFER_MANAGER_H_
#define FILE__BUFFER_MANAGER_H_

#include <map>
#include <string>

#include "file/file_manager.h"

class Page;
class Buffer;

class BufferManager {
friend class FileManager;
public:
    // static void flush(); // TODO: should be automatically called, move to destructor?
    static Page& get_page(uint_fast32_t page_number, FileId file_id);
    static Page& append_page(FileId file_id);

private:
    static BufferManager instance;
    std::map<std::pair<FileId, int>, int> pages;

    Page* buffer_pool;
    char* bytes;
    int clock_pos;

    BufferManager();
    ~BufferManager();

    bool flushed_at_exit = false;
    void _flush();
    int get_buffer_available();
    Page& _get_page(uint_fast32_t page_number, FileId file_id);
    Page& _append_page(FileId file_id);
};

#endif //FILE__BUFFER_MANAGER_H_
