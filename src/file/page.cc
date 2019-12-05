#include "file/page.h"

Page::Page(uint_fast32_t page_number, char* bytes, FileId file_id)
    : page_number(page_number), file_id(file_id), pins(1), dirty(false), bytes(bytes)
{
}

Page::Page()
    : page_number(0), file_id(FileId(FileId::UNASSIGNED)), pins(0), dirty(false), bytes(nullptr)
{
}

Page::~Page() {
    /*if (pins > 0) {
        std::cout << "ERROR: Destroying pinned page, this is a bug.\n";
    }
    flush();*/
}

Page& Page::operator=(const Page& other) {
    this->flush();
    this->page_number = other.page_number;
    this->file_id = other.file_id;
    this->pins = other.pins;
    this->dirty = other.dirty;
    this->bytes = other.bytes;
    return *this;
}

uint_fast32_t Page::get_page_number() const {
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

char* Page::get_bytes() const {
    return bytes;
}

void Page::flush() {
    if (dirty) {
        // std::cout << "Page::flush() " << page_number << ", " << filename << "\n";
        FileManager::flush(file_id, page_number, bytes);
        dirty = false;
    }
}
