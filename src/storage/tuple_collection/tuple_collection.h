#ifndef STORAGE__TUPLE_COLLECTION_H_
#define STORAGE__TUPLE_COLLECTION_H_

#include <iostream>
#include <string>
#include <vector>

#include "storage/page.h"
#include "storage/page_id.h"

class MergeOrderedTupleCollection;

class TupleCollection {
friend class MergeOrderedTupleCollection;
public:
    TupleCollection(Page& page, const uint64_t tuple_size);
   ~TupleCollection();
    std::vector<uint64_t> get(int n) const;
    bool is_full() const;
    void add(std::vector<uint64_t> new_tuple);
    void sort(bool (*is_leq)(std::vector<uint64_t> x, std::vector<uint64_t> y));
    const uint64_t tuple_size;

private:
    Page& page;
    uint64_t* tuple_count;
    uint64_t* tuples;

    void swap(int x, int y);
    void override_tuple(std::vector<uint64_t>& bytes, int position);
    int partition(int i, int f, bool (*is_leq)(std::vector<uint64_t> x, std::vector<uint64_t> y));
    void quicksort(int i, int f, bool (*is_leq)(std::vector<uint64_t> x, std::vector<uint64_t> y));

};

class MergeOrderedTupleCollection {
};

#endif  // STORAGE__TUPLE_COLLECTION_H_
