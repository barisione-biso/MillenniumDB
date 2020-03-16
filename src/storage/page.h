#ifndef STORAGE__PAGE_H_
#define STORAGE__PAGE_H_

#include <iostream>
#include <string>

#include "storage/file_manager.h"

const int PAGE_SIZE = 4096;

class Page {
friend class BufferManager;
public:
    void unpin();
    void make_dirty();
    char* get_bytes() const;
    uint_fast32_t get_page_number() const;

    Page& operator=(const Page& other);

private:
    FileId file_id;
    uint_fast32_t page_number;
    uint_fast32_t pins;
    char* bytes;
    bool dirty;

    Page();
    Page(uint_fast32_t page_number, char* bytes, FileId file_id);
    ~Page();

    void pin(); // Only buffer manager should call pin()
    void flush();
};

#endif // STORAGE__PAGE_H_
