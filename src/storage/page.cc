#include "page.h"

#include "storage/file_id.h"

Page::Page(uint_fast32_t page_number, char* bytes, FileId file_id)
    : file_id(file_id), page_number(page_number), pins(1), bytes(bytes), dirty(false) { }


Page::Page()
    : file_id(FileId(FileId::UNASSIGNED)), page_number(0), pins(0), bytes(nullptr), dirty(false) { }


Page::~Page() = default;


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
        file_manager.flush(file_id, page_number, bytes);
        dirty = false;
    }
}
