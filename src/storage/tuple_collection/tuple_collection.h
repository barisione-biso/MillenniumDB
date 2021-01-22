// A TupleCollection allows you to store arrangements
// of GraphObjects on disk, the purpose of this class is to abstract the
// operations of saving and reading the tuples on disk that a physical operator requires.

// TupleCollection asume that all the arrays of GraphObject have the same size


#ifndef STORAGE__TUPLE_COLLECTION_H_
#define STORAGE__TUPLE_COLLECTION_H_

#include <iostream>
#include <string>
#include <vector>

#include "base/graph/graph_object.h"
#include "storage/file_id.h"
#include "storage/page_id.h"
#include "storage/page.h"

class MergeOrderedTupleCollection;

class TupleCollection {
friend class MergeOrderedTupleCollection;
public:
    static constexpr size_t GRAPH_OBJECT_SIZE = sizeof(GraphObject);

    TupleCollection(Page& page, const size_t tuple_size);
    ~TupleCollection();
    bool is_full() const;
    uint64_t get_n_tuples() const;
    void add(std::vector<GraphObject> new_tuple);
    std::vector<GraphObject> get(uint_fast64_t n) const;
    void sort(std::vector<uint_fast64_t> order_vars, bool ascending);
    static bool has_priority(std::vector<GraphObject> lhs, std::vector<GraphObject> rhs, std::vector<uint_fast64_t> order_vars, bool ascending);
    void reset();

private:
    Page& page;
    const size_t tuple_size;
    uint64_t* tuple_count;
    GraphObject* tuples;
    void swap(int x, int y);
    void override_tuple(std::vector<GraphObject> bytes, int position);
    int partition(int i, int f, std::vector<uint_fast64_t> order_vars, bool ascending);
    void quicksort(int i, int f, std::vector<uint_fast64_t> order_vars, bool ascending);

};


// MergeOrderedTupleCollection abstract the merge of two
// sorted runs (arrays of tuple collections)


class MergeOrderedTupleCollection {
public:
    MergeOrderedTupleCollection(
        size_t tuple_size,
        std::vector<uint64_t> order_vars,
        bool ascending);

    void merge(
        uint_fast64_t left_start,
        uint_fast64_t left_end,
        uint_fast64_t right_start,
        uint_fast64_t right_end,
        FileId source_file_id,
        FileId output_file_id);

    void copy_page(
        uint_fast64_t source_page,
        FileId source_file_id,
        FileId output_file_id);

private:
    size_t tuple_size;
    std::vector<uint_fast64_t> order_vars;
    bool ascending;
    std::vector<GraphObject> left_tuple;
    std::vector<GraphObject> right_tuple;
};

#endif  // STORAGE__TUPLE_COLLECTION_H_
