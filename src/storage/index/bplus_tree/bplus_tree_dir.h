#ifndef STORAGE__INDEX__B_PLUS_TREE_DIR_H_
#define STORAGE__INDEX__B_PLUS_TREE_DIR_H_

#include <memory>

#include "storage/index/bplus_tree/bplus_tree_leaf.h"
#include "storage/index/bplus_tree/bplus_tree_split.h"
#include "storage/index/record.h"
#include "storage/page.h"

template <std::size_t N>
class BPlusTreeDir {
    template <std::size_t M> friend class BPlusTree;

    public:
        BPlusTreeDir(FileId const leaf_file_id, Page& page);
        ~BPlusTreeDir();

        std::unique_ptr<BPlusTreeSplit<N>> bulk_insert(BPlusTreeLeaf<N>& leaf);

        // returns not null when it needs to split
        std::unique_ptr<BPlusTreeSplit<N>> insert(const Record<N>& record);

        // std::unique_ptr<Record<N>> get(const Record<N>& key);
        SearchLeafResult search_leaf(const Record<N>& min) const noexcept;

        bool check() const;

    private:
        FileId const dir_file_id;
        FileId const leaf_file_id;
        Page& page;
        uint32_t* const key_count;
        uint64_t* const keys;
        int32_t* const children;

        int search_child_index(int from, int to, const Record<N>& record) const;
        void shift_right_keys(int from, int to);
        void shift_right_children(int from, int to);
        void update_key(int index, const Record<N>& record);
        void update_child(int index, int dir);
        void split(const Record<N>& record);
};

#endif // STORAGE__INDEX__B_PLUS_TREE_DIR_H_
