#ifndef STORAGE__TUPLE_COLLECTION_H_
#define STORAGE__TUPLE_COLLECTION_H_

#include <iostream>
#include <string>
#include <vector>

#include "storage/page.h"
#include "storage/page_id.h"

class TupleCollection {
public:
    TupleCollection(Page& page, const uint64_t tuple_size);
    std::vector<uint64_t> get(int n);
    bool is_full();
    void add(std::vector<uint64_t> new_tuple);
    void swap(int x, int y);
    void override_tuple(std::vector<uint64_t>& bytes, int position);
    void sort(bool (*is_leq)(std::vector<uint64_t> x, std::vector<uint64_t> y));
    const uint64_t tuple_size;
    uint64_t* tuple_count;
    uint64_t* tuples;
   ~TupleCollection();
private:
    Page& page;
    int partition(int i, int f, bool (*is_leq)(std::vector<uint64_t> x, std::vector<uint64_t> y));
    void quicksort(int i, int f, bool (*is_leq)(std::vector<uint64_t> x, std::vector<uint64_t> y));

};

#endif  // STORAGE__TUPLE_COLLECTION_H_
