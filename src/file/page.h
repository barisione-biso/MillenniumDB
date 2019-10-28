#ifndef PAGE_SIZE
    #define PAGE_SIZE 4096
#endif

#ifndef FILE__PAGE_H_
#define FILE__PAGE_H_

#include <iostream>
#include <string>
#include <fstream>

class Page {
    friend class BufferManager;
    public:
        uint_fast32_t page_number;
        std::string filename;

        void unpin();
        void reuse(uint_fast32_t page_number, const std::string& filename);
        void make_dirty();
        char* get_bytes();
    private:
        uint_fast32_t pins;
        bool dirty;
        char* const bytes;

        Page(uint_fast32_t page_number, char* bytes, const std::string& filename);
        ~Page();

        void pin(); // Only buffer manager should call pin()
        void flush();
};

#endif //FILE__PAGE_H_
