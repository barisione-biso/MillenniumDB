#ifndef STORAGE__B_PLUS_TREE_LEAF_H_
#define STORAGE__B_PLUS_TREE_LEAF_H_

#include <memory>

#include "storage/page.h"
#include "storage/index/record.h"
#include "storage/index/bplus_tree/bplus_tree_split.h"

// forward declarations for friend
template <std::size_t N> class BPlusTreeDir;
template <std::size_t N> class BPlusTree;


struct SearchLeafResult {
    int page_number;
    int result_index;

    SearchLeafResult(int page_number, int result_index)
        : page_number(page_number), result_index(result_index) { }
};


template <std::size_t N>
class BPlusTreeLeaf {
    template <std::size_t M> friend class BPlusTreeDir;
    template <std::size_t M> friend class BPlusTree;

    public:
        BPlusTreeLeaf(Page& page);
        ~BPlusTreeLeaf();

        std::unique_ptr<BPlusTreeSplit<N>> insert(const Record<N>& record);
        SearchLeafResult search_leaf(const Record<N>& min);

        // std::unique_ptr<Record> get(const Record& key);

        void create_new(const Record<N>& record);

        bool is_leaf()  { return true; }
        uint32_t get_value_count() { return value_count; }
        int has_next()  { return next_leaf != 0; }

        bool check() const;
        void print() const;

        std::unique_ptr<BPlusTreeLeaf> get_next_leaf();
        std::unique_ptr<Record<N>> get_record(int pos);

    private:
        Page& page;
        FileId leaf_file_id;
        uint32_t value_count;
        uint32_t next_leaf;
        uint64_t* records;

        uint_fast32_t search_index(int from, int to, const Record<N>& record);
        bool equal_record(const Record<N>& record, uint_fast32_t index);
        void shift_right_records(uint_fast32_t from, uint_fast32_t to);
};

template class BPlusTreeLeaf<2>;
template class BPlusTreeLeaf<3>;
template class BPlusTreeLeaf<4>;

#endif // STORAGE__B_PLUS_TREE_LEAF_H_
