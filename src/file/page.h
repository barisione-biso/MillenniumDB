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
        void unpin();
        // void reuse(int page_number, const std::string& filename);
        void make_dirty();
        char* get_bytes();
        uint_fast32_t get_page_number();
    private:
        uint_fast32_t page_number;
        const std::string& filename;
        uint_fast32_t pins;
        bool dirty;
        char* const bytes;

        Page(int page_number, char* bytes, const std::string& filename);
        ~Page();

        void pin(); // Only buffer manager should call pin()
        void flush();
};

#endif //FILE__PAGE_H_
