#ifndef PAGE_SIZE
    #define PAGE_SIZE 4096
#endif

#ifndef FILE__PAGE_H_
#define FILE__PAGE_H_

#include <iostream>
#include <string>
#include <fstream>

#include "file/buffer.h"

class Page {
    friend class BufferManager;
    public:
        const uint_fast32_t page_number;
        const std::string filename;

        void unpin();
        void make_dirty();
        char* get_bytes();
    private:
        uint_fast32_t pins;
        bool dirty;
        Buffer &buffer;

        Page(uint_fast32_t page_number, Buffer& buffer, const std::string& filename);
        ~Page();

        void pin(); // Only buffer manager should call pin()

        void flush();
};

#endif //FILE__PAGE_H_
