#ifndef STORAGE__TUPLE_COLLECTION_H_
#define STORAGE__TUPLE_COLLECTION_H_

#include <iostream>
#include <string>
#include <vector>

#include "storage/page.h"
#include "storage/page_id.h"
#include "storage/file_id.h"

class MergeOrderedTupleCollection;

class TupleCollection {
friend class MergeOrderedTupleCollection;
public:
    TupleCollection(Page& page, const size_t tuple_size);
   ~TupleCollection();
    std::vector<uint64_t> get(uint_fast64_t n) const;
    bool is_full() const;
    uint64_t get_n_tuples() const;
    void add(std::vector<uint64_t> new_tuple);
    void sort(bool (*is_leq)(std::vector<uint64_t> x, std::vector<uint64_t> y, std::vector<uint64_t> order_vars),std::vector<uint64_t> order_vars);
    void reset();

private:
    Page& page;
    const size_t tuple_size;
    uint64_t* tuple_count;
    uint64_t* tuples;
    void swap(int x, int y);
    void override_tuple(std::vector<uint64_t>& bytes, int position);
    int partition(int i, int f, bool (*is_leq)(std::vector<uint64_t> x, std::vector<uint64_t> y, std::vector<uint64_t> order_vars), std::vector<uint64_t> order_vars);
    void quicksort(int i, int f, bool (*is_leq)(std::vector<uint64_t> x, std::vector<uint64_t> y, std::vector<uint64_t> order_vars), std::vector<uint64_t> order_vars);

};

class MergeOrderedTupleCollection {
public:
    MergeOrderedTupleCollection(
        size_t tuple_size,
        std::vector<uint64_t> order_vars,
        bool (*is_leq)(std::vector<uint64_t> x, std::vector<uint64_t> y, std::vector<uint64_t> order_vars));
    void merge(
        uint_fast64_t left_start,
        uint_fast64_t left_end,
        uint_fast64_t right_start,
        uint_fast64_t right_end,
        FileId source_file_id,
        FileId output_file_id
        );
    void copy_page(
      uint_fast64_t source_page,
      FileId source_file_id,
      FileId output_file_id
    );
private:
    size_t tuple_size;
    std::vector<uint_fast64_t> order_vars;
    bool (*has_priority)(std::vector<uint64_t> x, std::vector<uint64_t> y, std::vector<uint64_t> order_vars);
    std::vector<uint_fast64_t> left_tuple;
    std::vector<uint_fast64_t> right_tuple;
};

#endif  // STORAGE__TUPLE_COLLECTION_H_
