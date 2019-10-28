#include "file/page.h"

Page::Page(uint_fast32_t page_number, char* bytes, const std::string& filename)
    : page_number(page_number), filename(filename), pins(1), dirty(false), bytes(bytes)
{
}

Page::~Page(){
    // std::cout << "destroying page (" << page_number << ", " << filename << ")\n";
}

void Page::reuse(uint_fast32_t page_number, const std::string& filename) {
    this->filename = filename;
    this->page_number = page_number;
    this->pins = 1;
    this->dirty = false;
    // this->bytes are expected to be rewritten later
}

void Page::unpin() {
    pins--;
}

void Page::make_dirty() {
    dirty = true;
}

void Page::pin() {
    pins++;
}

char* Page::get_bytes() {
    return bytes;
}

void Page::flush() {
    if (dirty) {
        // std::cout << "Page::flush() " << page_number << ", " << filename << "\n";
        std::fstream file;
        file.open(filename, std::fstream::in|std::fstream::out|std::fstream::binary);
        file.seekp(page_number*PAGE_SIZE);
        // std::cout << file.tellp() << "\n";
        file.write(bytes, PAGE_SIZE);
        file.close();
        dirty = false;
    }
}
