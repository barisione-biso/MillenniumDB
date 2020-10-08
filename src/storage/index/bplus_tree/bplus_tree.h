/*
 * BPlusTree allows to search a record in logarithmic time
 *
 * example usage: // TODO: update
 * // search all records between (0,0) and (20, 1000)
 * // (including equal records)
 * auto iter = bplus_tree.get_range(Record(0, 0), Record(20, 1000));
 * auto record = iter->next();
 * while (record != nullptr) {
 *     // record is a not-null unique_ptr<Record>
 *     // do some work with record
 *     record = iter->next();
 * }
 * */

#ifndef STORAGE__B_PLUS_TREE_H_
#define STORAGE__B_PLUS_TREE_H_

#include <string>
#include <memory>

#include "storage/file_id.h"
#include "storage/index/record.h"
#include "storage/index/bplus_tree/bplus_tree_dir.h"
#include "storage/index/bplus_tree/bplus_tree_leaf.h"

class BptLeafProvider;

template <std::size_t N> class BptIter {
public:
    BptIter(FileId leaf_file_id, int leaf_page_number, int current_pos, const Record<N>& max);
    ~BptIter() = default;
    std::unique_ptr<Record<N>> next();

private:
    const FileId leaf_file_id;
    const Record<N> max;
    uint32_t current_pos;
    std::unique_ptr<BPlusTreeLeaf<N>> current_leaf;
};


template <std::size_t N> class BPlusTree {
public:
    // (PAGE_SIZE - SIZE_OF(value_count) - SIZE_OF(next_leaf)) / (SIZE_OF(UINT64) * N)
    static constexpr auto leaf_max_records = (PAGE_SIZE - 2*sizeof(int32_t) ) / (sizeof(uint64_t)*N);
    static constexpr auto dir_max_records  = (PAGE_SIZE - 2*sizeof(int32_t) ) / (sizeof(uint64_t)*N + sizeof(int32_t));

    BPlusTree(const std::string& name);
    ~BPlusTree() = default;

    const FileId dir_file_id;
    const FileId leaf_file_id;

    void bulk_import(BptLeafProvider&);
    void insert(const Record<N>& record);
    // std::unique_ptr<Record<N>> get(const Record<N>& record);
    bool check() const;

    std::unique_ptr<BptIter<N>> get_range(const Record<N>& min, const Record<N>& max) const;

private:
    bool is_empty;
    BPlusTreeDir<N> root;
    void create_new(const Record<N>& record);
};

#endif // STORAGE__B_PLUS_TREE_H_
