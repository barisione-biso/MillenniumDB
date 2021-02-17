#include "buffer_manager.h"

#include <cassert>
#include <new>         // placement new
#include <type_traits> // aligned_storage

#include "storage/file_manager.h"
#include "storage/page.h"

using namespace std;

// memory for the object
static typename std::aligned_storage<sizeof(BufferManager), alignof(BufferManager)>::type buffer_manager_buf;
// global object
BufferManager& buffer_manager = reinterpret_cast<BufferManager&>(buffer_manager_buf);


BufferManager::BufferManager(uint_fast32_t buffer_pool_size) :
    buffer_pool_size (buffer_pool_size),
    buffer_pool      (new Page[buffer_pool_size]),
    bytes            (new char[buffer_pool_size*Page::PAGE_SIZE]),
    clock_pos        (0) { }


BufferManager::~BufferManager() {
    flush();
    delete[](bytes);
    delete[](buffer_pool);
}

void BufferManager::init(uint_fast32_t buffer_pool_size) {
    new (&buffer_manager) BufferManager(buffer_pool_size); // placement new
}


void BufferManager::flush() {
    // flush() is always called at destruction.
    // this is important to check to avoid segfault when program terminates before calling init()
    assert(buffer_pool != nullptr);
    for (uint_fast32_t i = 0; i < buffer_pool_size; i++) {
        buffer_pool[i].flush();
    }
}


Page& BufferManager::get_last_page(FileId file_id) {
    auto page_count = file_manager.count_pages(file_id);
    if (page_count == 0) {
        return get_page(file_id, 0);
    } else {
        return get_page(file_id, page_count-1);
    }
}


Page& BufferManager::append_page(FileId file_id) {
    return get_page(file_id, file_manager.count_pages(file_id));
}


uint_fast32_t BufferManager::get_buffer_available() {
    auto first_lookup = clock_pos;

    while (buffer_pool[clock_pos].pins != 0) {
        clock_pos = (clock_pos+1)%buffer_pool_size;
        if (clock_pos == first_lookup) {
            throw std::runtime_error("No buffer available in buffer pool.");
        }
    }
    auto res = clock_pos;
    clock_pos = (clock_pos+1)%buffer_pool_size;
    return res;
}


Page& BufferManager::get_page(FileId file_id, uint_fast32_t page_number) noexcept {
    const PageId page_id(file_id, page_number);

    std::lock_guard<std::mutex> lck(pin_mutex);
    auto it = pages.find(page_id);

    if (it == pages.end()) {
        const auto buffer_available = get_buffer_available();
        if (buffer_pool[buffer_available].page_id.file_id.id != FileId::UNASSIGNED) {
            auto old_page_id = buffer_pool[buffer_available].page_id;
            pages.erase(old_page_id);
        }
        buffer_pool[buffer_available] = Page(page_id, &bytes[buffer_available*Page::PAGE_SIZE]);

        file_manager.read_page(page_id, buffer_pool[buffer_available].get_bytes());
        pages.insert(pair<PageId, int>(page_id, buffer_available));
        return buffer_pool[buffer_available];
    } else { // page is the buffer
        buffer_pool[it->second].pins++;
        return buffer_pool[it->second];
    }
    // lock is released
}


void BufferManager::unpin(Page& page) {
    std::lock_guard<std::mutex> lck(pin_mutex);
    assert(page.pins != 0 && "Must not unpin if pin count is equal to 0. There is a logic error.");
    page.pins--;
}


void BufferManager::remove(FileId file_id) {
    assert(buffer_pool != nullptr);
    for (uint_fast32_t i = 0; i < buffer_pool_size; i++) {
        if (buffer_pool[i].page_id.file_id == file_id) {
            pages.erase(buffer_pool[i].page_id);
            buffer_pool[i].reset();
        }
    }
}
