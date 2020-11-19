/* SortPage represents the content of a disk block in memory.
 * A page is treated as an array of `PAGE_SIZE` bytes (pointed by `bytes`).
 * For better performance, `PAGE_SIZE` should be multiple of the operating system's page size.
 * BufferManager is the only class who can construct a SortPage object. Other classes must get a SortPage
 * through BufferManager.
 */
#ifndef STORAGE__SORT_PAGE_H_
#define STORAGE__SORT_PAGE_H_

#include <iostream>
#include <string>
#include <vector>

#include "storage/page.h"
#include "storage/page_id.h"

class SortPage {
friend class BufferManager; // needed to access private constructor
public:
    SortPage(Page& page);
    uint_fast64_t get_pageno();
    uint_fast16_t get_dirsize();
    uint_fast16_t get_freespace();
    bool has_space_for(uint_fast64_t space);
    void create_header(uint_fast64_t pageno);
    std::vector<uint_fast64_t> get(int n);
    void write(std::vector<uint_fast64_t>& bytes);
    void make_dirty();
    void unpin();
    /*
    void flush() const { page.flush(); };
    void unpin() const { page.unpin(); };
    */
    // mark as dirty so when page is replaced it is written back to disk.
    // only meant for buffer_manager.remove()
    //void reset();

    // get the start memory position of `PAGE_SIZE` allocated bytes
    //inline char* get_bytes() const { return bytes; };

    // get page number
    // inline uint_fast32_t get_page_number() const { return page_id.page_number; };

private:
    Page& page;
    uint_fast32_t pins;
    int partition(int i, int f, bool (*is_leq)(std::vector<uint_fast64_t> x, std::vector<uint_fast64_t> y));
    void quicksort(int i, int f, bool (*is_leq)(std::vector<uint_fast64_t> x, std::vector<uint_fast64_t> y));
    void swap(int x, int y);
    void override_tuple(std::vector<uint_fast64_t>& bytes, int position);
    SortPage();
    ~SortPage();
};

#endif
