#ifndef STORAGE__B_PLUS_TREE_LEAF_H_
#define STORAGE__B_PLUS_TREE_LEAF_H_

#include <memory>

#include "storage/index/bplus_tree/bplus_tree_split.h"
#include "storage/index/record.h"
#include "storage/page.h"

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
        SearchLeafResult search_leaf(const Record<N>& min) const noexcept;

        // std::unique_ptr<Record> get(const Record& key);

        void create_new(const Record<N>& record);

        uint32_t get_value_count() const { return *value_count; }
        bool has_next()            const { return *next_leaf != 0; }

        bool check() const;
        void print() const;

        Page& get_page() const noexcept { return page; }

        std::unique_ptr<BPlusTreeLeaf> get_next_leaf() const;
        std::unique_ptr<Record<N>> get_record(uint_fast32_t pos) const;

    private:
        Page& page;
        FileId leaf_file_id;
        uint32_t* const value_count;
        uint32_t* const next_leaf;
        uint64_t* const records;

        uint_fast32_t search_index(int from, int to, const Record<N>& record) const;
        bool equal_record(const Record<N>& record, uint_fast32_t index);
        void shift_right_records(int from, int to);
};

#endif // STORAGE__B_PLUS_TREE_LEAF_H_
