#ifndef PAGE_SIZE
    #define PAGE_SIZE 4096
#endif

#ifndef FILE__PAGE_H_
#define FILE__PAGE_H_

#include <iostream>
#include <string>

#include "file/file_manager.h"

class Page {
    friend class BufferManager;
    public:
        void unpin();
        void make_dirty();
        char* get_bytes() const;
        uint_fast32_t get_page_number() const;

        Page& operator=(const Page& other);
    private:
        uint_fast32_t page_number;
        FileId file_id;
        uint_fast32_t pins;
        bool dirty;
        char* bytes;

        Page();
        Page(uint_fast32_t page_number, char* bytes, FileId file_id);
        ~Page();

        void pin(); // Only buffer manager should call pin()
        void flush();
};

#endif //FILE__PAGE_H_
