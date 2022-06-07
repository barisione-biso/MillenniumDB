/* Page represents the content of a disk block in memory.
 * A page is treated as an array of `MDB_PAGE_SIZE` bytes (pointed by `bytes`).
 * For better performance, `MDB_PAGE_SIZE` should be multiple of the operating system's page size.
 * BufferManager is the only class who can construct a Page object. Other classes must get a Page
 * through BufferManager.
 */
#pragma once

#include <cassert>

#include "storage/page_id.h"

class Page {
friend class BufferManager;
public:
    static constexpr auto MDB_PAGE_SIZE = 4096;

    // contains file_id and page_number of this page
    PageId page_id;

    // mark as dirty so when page is replaced it is written back to disk.
    inline void make_dirty() noexcept { dirty = true; }

    // get the start memory position of `MDB_PAGE_SIZE` allocated bytes
    inline char* get_bytes() const noexcept { return bytes; }

    // get page number
    inline uint32_t get_page_number() const noexcept { return page_id.page_number; };

private:
    // count of objects using this page, modified only by buffer_manager
    uint32_t pins;

    // start memory address of the page, of size `MDB_PAGE_SIZE`
    char* bytes;

    // true if data in memory is different from disk
    bool dirty;

    Page() noexcept :
        page_id(FileId(FileId::UNASSIGNED), 0),
        pins(0),
        bytes(nullptr),
        dirty(false) { }

    Page(PageId page_id, char* bytes) noexcept:
        page_id(page_id),
        pins(1),
        bytes(bytes),
        dirty(false) { }

    // only meant for buffer_manager.remove()
    void reset() noexcept {
        assert(pins == 0 && "Cannot reset page if it is pinned");
        this->page_id = PageId(FileId(FileId::UNASSIGNED), 0);
        this->pins    = 0;
        this->dirty   = false;
        this->bytes   = nullptr;
    }

    void operator=(const Page& other) noexcept {
        assert(pins == 0 && "Cannot reassign page if it is pinned");
        this->page_id = other.page_id;
        this->pins    = other.pins;
        this->dirty   = other.dirty;
        this->bytes   = other.bytes;
    }
};
