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
    BPlusTreeLeaf(BPlusTreeParams& params, Page& page);
    ~BPlusTreeLeaf();
    std::unique_ptr<std::pair<Record, int>> insert(Record& record);
    std::pair<int, int> search_leaf(const Record& min);

    void create_new(Record& record); // method used for inserting the first record of the B+Tree
    bool is_leaf() { return true; }
    int get_count() { return *count; }
    int has_next() { return *next != 0; }
    std::unique_ptr<BPlusTreeLeaf> next_leaf();
    Record get_record(int pos);
    static inline int instance_count = 0;

private:
    BPlusTreeParams& params;
    Page& page;
    int* count;
    int* next;
    uint64_t* records;

    int search_index(int from, int to, const Record& record);
};

#endif //FILE__INDEX__B_PLUS_TREE__B_PLUS_TREE_LEAF_