#include "file/page.h"

Page::Page(uint_fast32_t page_number, Buffer& buffer, const std::string& filename)
    : page_number(page_number), filename(filename), pins(1), dirty(false), buffer(buffer)
{
}

Page::~Page(){
    // std::cout << "destroying page (" << page_number << ", " << filename << ")\n";
}

void Page::unpin() {
    pins--;
}

void Page::make_dirty() {
    dirty = true;
}

char* Page::get_bytes() {
    return buffer.get_bytes();
}

void Page::pin() {
    pins++;
}

void Page::flush() {
    if (dirty) {
        // std::cout << "Page::flush() " << page_number << ", " << filename << "\n";
        std::fstream file;
        file.open(filename, std::fstream::in|std::fstream::out|std::fstream::binary);
        file.seekp(page_number*PAGE_SIZE);
        // std::cout << file.tellp() << "\n";
        file.write(buffer.get_bytes(), PAGE_SIZE);
        file.close();
    }
}
