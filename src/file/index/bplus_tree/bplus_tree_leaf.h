#ifndef FILE__INDEX__B_PLUS_TREE__B_PLUS_TREE_LEAF_
#define FILE__INDEX__B_PLUS_TREE__B_PLUS_TREE_LEAF_

#include <iostream>
#include <memory>
#include <tuple>

class Page;
class Record;
class BPlusTreeParams;

class BPlusTreeLeaf {
friend class BPlusTree;
public:
    BPlusTreeLeaf(const BPlusTreeParams& params, Page& page);
    ~BPlusTreeLeaf();

    std::unique_ptr<std::pair<Record, int>> insert(const Record& key, const Record& value);
    std::pair<int, int> search_leaf(const Record& min);

    void edit(const Record& key, const Record& value);
    std::unique_ptr<Record> get(const Record& key);

    void create_new(const Record& key, const Record& value);

    bool is_leaf()  { return true; }
    int get_count() { return *count; }
    int has_next()  { return *next != 0; }
    void print() const;

    std::unique_ptr<BPlusTreeLeaf> next_leaf();
    std::unique_ptr<Record> get_record(int pos);

private:
    const BPlusTreeParams& params;
    Page& page;
    int* count;
    int* next;
    uint64_t* records;

    int search_index(int from, int to, const Record& record);
    bool equal_record(const Record& record, int index);
    void shift_right_records(int from, int to);
};

#endif //FILE__INDEX__B_PLUS_TREE__B_PLUS_TREE_LEAF_