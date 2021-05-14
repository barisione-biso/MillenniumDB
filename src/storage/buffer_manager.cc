#include "buffer_manager.h"

#include <cassert>
#include <new>         // placement new
#include <type_traits> // aligned_storage
#include <iostream>

#include "storage/file_manager.h"

using namespace std;

// memory for the object
static typename std::aligned_storage<sizeof(BufferManager), alignof(BufferManager)>::type buffer_manager_buf;
// global object
BufferManager& buffer_manager = reinterpret_cast<BufferManager&>(buffer_manager_buf);


BufferManager::BufferManager(uint_fast32_t buffer_pool_size) :
    buffer_pool_size          (buffer_pool_size),
    private_buffer_pool_size  (buffer_pool_size/3),  // TODO: change this to parameter
    max_private_buffers       (10),  // TODO: change this to parameter
    buffer_pool               (new Page[buffer_pool_size]),
    private_buffer_pool       (new Page[private_buffer_pool_size * max_private_buffers]),
    bytes                     (new char[buffer_pool_size*Page::PAGE_SIZE]),
    private_bytes             (new char[private_buffer_pool_size*max_private_buffers*Page::PAGE_SIZE]),
    clock_pos                 (0)
    {
        for (uint_fast32_t i=0; i < max_private_buffers; i++) {
            available_private_positions.push(i);
        }
        private_tmp_pages.resize(max_private_buffers);
        private_clock_pos.resize(max_private_buffers);
    }


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


uint_fast32_t BufferManager::get_private_buffer_available(uint_fast32_t thread_pos) {
    // analogous to shared buffer
    auto first_lookup = private_clock_pos[thread_pos];

    while (get_private_page(thread_pos, private_clock_pos[thread_pos]).pins != 0) {
        private_clock_pos[thread_pos] = (private_clock_pos[thread_pos]+1)%private_buffer_pool_size;
        if (private_clock_pos[thread_pos] == first_lookup) {
            throw std::runtime_error("No buffer available in private buffer pool.");
        }
    }
    auto ret = private_clock_pos[thread_pos];
    private_clock_pos[thread_pos] = (private_clock_pos[thread_pos]+1)%private_buffer_pool_size;
    return ret;
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


Page& BufferManager::get_tmp_page(TmpFileId file_id, uint_fast32_t page_number) {
    const PageId page_id(FileId(file_id.id), page_number);
    // get thread id
    thread::id this_id = std::this_thread::get_id();
    // get private thread pos
    auto thread_pos_it = thread2index.find(this_id);
    if (thread_pos_it == thread2index.end()) {
        // new thread
        std::cout << "thread: " << this_id << "\n";
        if (available_private_positions.empty()) {
            throw std::runtime_error("To many threads, not enough private buffer space");
        }
        uint_fast32_t new_thread_pos = available_private_positions.front();
        available_private_positions.pop();
        thread2index.insert(pair<thread::id, uint_fast32_t>(this_id, new_thread_pos));
        private_tmp_pages[new_thread_pos].clear();
        private_clock_pos[new_thread_pos] = 0;
        auto& page = get_private_page(new_thread_pos, 0);
        page = Page(
            page_id,
            &private_bytes[Page::PAGE_SIZE * (new_thread_pos * private_buffer_pool_size)]
        );
        return page;
    }
    else {
        // old thread
        auto thread_pos = thread_pos_it->second;
        auto pages_it = private_tmp_pages[thread_pos].find(page_id);
        if (pages_it == private_tmp_pages[thread_pos].end()) {
            // TODO: handle exception
            const auto buffer_available = get_private_buffer_available(thread_pos);
            auto& page = get_private_page(thread_pos, buffer_available);
            if (page.page_id.file_id.id != TmpFileId::UNASSIGNED) {
                private_tmp_pages[thread_pos].erase(page.page_id);
            }
            // TODO: test this
            page = Page(page_id,
                        &private_bytes[Page::PAGE_SIZE * ((thread_pos * private_buffer_pool_size) + buffer_available)]
            );
            file_manager.read_page(page_id, page.get_bytes());
            private_tmp_pages[thread_pos].insert(pair<PageId, int>(page_id, buffer_available));
            return page;
        }
        else {
            auto& page = get_private_page(thread_pos, pages_it->second);
            page.pins++;
            return page;
        }
    }
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
