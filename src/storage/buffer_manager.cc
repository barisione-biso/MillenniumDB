#include "buffer_manager.h"

#include <cstdlib>
#include <new>         // placement new
#include <type_traits> // aligned_storage

#include "base/exceptions.h"
#include "storage/file_manager.h"

using namespace std;

// memory for the object
static typename std::aligned_storage<sizeof(BufferManager), alignof(BufferManager)>::type buffer_manager_buf;
// global object
BufferManager& buffer_manager = reinterpret_cast<BufferManager&>(buffer_manager_buf);


BufferManager::BufferManager(uint_fast32_t shared_buffer_pool_size,
                             uint_fast32_t private_buffer_pool_size,
                             uint_fast32_t max_threads) :
    shared_buffer_pool_size   (shared_buffer_pool_size),
    private_buffer_pool_size  (private_buffer_pool_size),
    max_private_buffers       (max_threads),
    buffer_pool               (new Page[shared_buffer_pool_size]),
    private_buffer_pool       (new Page[private_buffer_pool_size * max_private_buffers]),
    bytes                     (reinterpret_cast<char*>(std::aligned_alloc(
                                   Page::MDB_PAGE_SIZE,
                                   shared_buffer_pool_size * Page::MDB_PAGE_SIZE))),
    private_bytes             (reinterpret_cast<char*>(std::aligned_alloc(
                                   Page::MDB_PAGE_SIZE,
                                   private_buffer_pool_size * max_private_buffers * Page::MDB_PAGE_SIZE))),
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


void BufferManager::init(uint_fast32_t shared_buffer_pool_size,
                         uint_fast32_t private_buffer_pool_size,
                         uint_fast32_t max_threads)
{
    // placement new
    new (&buffer_manager) BufferManager(shared_buffer_pool_size, private_buffer_pool_size, max_threads);
}


void BufferManager::flush() {
    // flush() is always called at destruction.
    // this is important to check to avoid segfault when program terminates before calling init()
    assert(buffer_pool != nullptr);
    for (uint_fast32_t i = 0; i < shared_buffer_pool_size; i++) {
        if (buffer_pool[i].dirty) {
            file_manager.flush(buffer_pool[i].page_id, bytes);
            buffer_pool[i].dirty = false;
        }
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
        clock_pos = (clock_pos+1) % shared_buffer_pool_size;
        if (__builtin_expect(!!(clock_pos == first_lookup), 0)) {
            throw QueryExecutionException("No buffer available in buffer pool");
        }
    }
    auto res = clock_pos;
    clock_pos = (clock_pos+1) % shared_buffer_pool_size;
    return res;
}


uint_fast32_t BufferManager::get_private_buffer_available(uint_fast32_t thread_pos) {
    // analogous to shared buffer
    auto first_lookup = private_clock_pos[thread_pos];

    while (get_private_page(thread_pos, private_clock_pos[thread_pos]).pins != 0) {
        private_clock_pos[thread_pos] = (private_clock_pos[thread_pos]+1)%private_buffer_pool_size;
        if (__builtin_expect(!!(private_clock_pos[thread_pos] == first_lookup), 0)) {
            throw QueryExecutionException("No buffer available in private buffer pool.");
        }
    }
    auto ret = private_clock_pos[thread_pos];
    private_clock_pos[thread_pos] = (private_clock_pos[thread_pos]+1)%private_buffer_pool_size;
    return ret;
}


Page& BufferManager::get_page(FileId file_id, uint_fast32_t page_number) noexcept {
    const PageId page_id(file_id, page_number);

    std::lock_guard<std::mutex> lck(shared_buffer_mutex);
    auto it = pages.find(page_id);

    if (it == pages.end()) {
        const auto buffer_available = get_buffer_available();
        auto& page = buffer_pool[buffer_available];
        if (page.page_id.file_id.id != FileId::UNASSIGNED) {
            pages.erase(page.page_id);
        }

        if (page.dirty) {
            file_manager.flush(page.page_id, bytes);
            page.dirty = false;
        }
        page = Page(page_id, &bytes[buffer_available*Page::MDB_PAGE_SIZE]);

        file_manager.read_page(page_id, page.get_bytes());
        pages.insert({page_id, &page});
        return page;
    } else { // page is the buffer
        it->second->pins++;
        return *it->second;
    }
    // lock is released
}


Page& BufferManager::get_tmp_page(TmpFileId tmp_file_id, uint_fast32_t page_number) {
    const PageId page_id(tmp_file_id.file_id, page_number);
    auto thread_pos = tmp_file_id.private_buffer_pos;

    // We don't need a mutex here because tmp pages are assigned to one specific thread
    // TODO: If we change this in the future we might need a mutex lock

    auto it = private_tmp_pages[thread_pos].find(page_id);
    if (it == private_tmp_pages[thread_pos].end()) {
        const auto buffer_available = get_private_buffer_available(thread_pos);
        auto& page = get_private_page(thread_pos, buffer_available);
        if (page.page_id.file_id.id != TmpFileId::UNASSIGNED) {
            private_tmp_pages[thread_pos].erase(page.page_id);
        }

        if (page.dirty) {
            file_manager.flush(page.page_id, bytes);
            page.dirty = false;
        }
        page = Page(page_id,
                    &private_bytes[Page::MDB_PAGE_SIZE * (thread_pos * private_buffer_pool_size + buffer_available)]);

        file_manager.read_page(page_id, page.get_bytes());
        private_tmp_pages[thread_pos].insert({page_id, &page});
        return page;
    }
    else {
        it->second->pins++;
        return *it->second;
    }
}


void BufferManager::remove(FileId file_id) {
    assert(buffer_pool != nullptr);
    for (uint_fast32_t i = 0; i < shared_buffer_pool_size; i++) {
        if (buffer_pool[i].page_id.file_id == file_id) {
            pages.erase(buffer_pool[i].page_id);
            buffer_pool[i].reset();
        }
    }
}


void BufferManager::remove_tmp(TmpFileId tmp_file_id) {
    assert(private_buffer_pool != nullptr);
    auto offset = private_buffer_pool_size * tmp_file_id.private_buffer_pos;
    for (uint_fast32_t i = 0; i < private_buffer_pool_size; i++) {
        auto page_id = private_buffer_pool[offset + i].page_id;
        if (page_id.file_id == tmp_file_id.file_id) {
            private_tmp_pages[tmp_file_id.private_buffer_pos].erase(page_id);
            private_buffer_pool[offset + i].reset();
        }
    }
}


uint_fast32_t BufferManager::get_private_buffer_index() {
    thread::id this_id = std::this_thread::get_id();
    auto thread_pos_it = thread2index.find(this_id);
    if (thread_pos_it == thread2index.end()) {
        // new thread
        if (available_private_positions.empty()) {
            throw QueryExecutionException("Too many threads using private buffer at the moment");
        }
        uint_fast32_t new_thread_pos = available_private_positions.front();
        available_private_positions.pop();
        thread2index.insert({this_id, new_thread_pos});
        private_tmp_pages[new_thread_pos].clear();
        private_clock_pos[new_thread_pos] = 0;
        return new_thread_pos;
    }
    else { // old thread
        return thread_pos_it->second;
    }
}
