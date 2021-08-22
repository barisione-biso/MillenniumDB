// A TupleCollection allows you to store arrangements
// of GraphObjects on disk, the purpose of this class is to abstract the
// operations of saving and reading the tuples on disk that a physical operator requires.

// TupleCollection asumes that all the arrays of GraphObject have the same size


#ifndef STORAGE__TUPLE_COLLECTION_H_
#define STORAGE__TUPLE_COLLECTION_H_

#include <iostream>
#include <string>
#include <vector>

#include "base/graph/graph_object.h"
#include "base/ids/var_id.h"
#include "storage/file_id.h"
#include "storage/page_id.h"
#include "storage/page.h"

class MergeOrderedTupleCollection;

class TupleCollection {
friend class MergeOrderedTupleCollection;
public:
    TupleCollection(Page& page, const size_t tuple_size);
    ~TupleCollection();

    bool is_full() const {
        return sizeof(tuple_count) + (sizeof(GraphObject)*tuple_size*(1 + *tuple_count)) > Page::MDB_PAGE_SIZE;
    }

    inline uint64_t get_tuple_count() const noexcept { return *tuple_count; }

    std::vector<GraphObject> get(uint64_t n) const;

    static bool has_priority(const std::vector<GraphObject>&  lhs,
                             const std::vector<GraphObject>&  rhs,
                             const std::vector<VarId>&        order_vars,
                             const std::vector<bool>&         ascending);

    void add(std::vector<GraphObject> new_tuple);
    void sort(std::vector<VarId>& order_vars, std::vector<bool>& ascending);
    void reset();

private:
    Page& page;
    const size_t tuple_size;
    GraphObject* const tuples;
    uint64_t* const tuple_count;

    void swap(int x, int y);
    void override_tuple(const std::vector<GraphObject>& tuple, int position);
    int partition(int i, int f, std::vector<VarId>& order_vars, std::vector<bool>& ascending);
    void quicksort(int i, int f, std::vector<VarId>& order_vars, std::vector<bool>& ascending);
};


// MergeOrderedTupleCollection abstract the merge of two
// sorted runs (arrays of tuple collections)
class MergeOrderedTupleCollection {
public:
    MergeOrderedTupleCollection(size_t tuple_size,
                                const std::vector<VarId>& order_vars,
                                const std::vector<bool>& ascending,
                                bool* interruption_requested);

    void merge(uint64_t  left_start,
               uint64_t  left_end,
               uint64_t  right_start,
               uint64_t  right_end,
               TmpFileId source_file_id,
               TmpFileId output_file_id);

    void copy_page(uint64_t  source_page,
                   TmpFileId source_file_id,
                   TmpFileId output_file_id);

private:
    const size_t              tuple_size;
    const std::vector<VarId>& order_vars;
    const std::vector<bool>&  ascending;
    bool const *              interruption_requested;
};

#endif // STORAGE__TUPLE_COLLECTION_H_
