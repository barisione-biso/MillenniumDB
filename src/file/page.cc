#include "file/page.h"

Page::Page(uint_fast32_t page_number, char* bytes, FileId file_id)
    : page_number(page_number), file_id(file_id), pins(1), dirty(false), bytes(bytes)
{
}

Page::~Page(){
    if (pins > 0) {
        std::cout << "Destroying pinned page\n";
    }
    flush();
    // std::cout << "destroying page (" << page_number << ", " << filename << ")\n";
}

uint_fast32_t Page::get_page_number() {
    return page_number;
}

void Page::unpin() {
    if (pins == 0) {
        throw std::logic_error("Inconsistent unpin when pins == 0");
    }
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
        FileManager::flush(file_id, page_number, bytes);
        dirty = false;
    }
}
