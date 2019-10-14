#ifndef FILE__INDEX__B_PLUS_TREE__B_PLUS_TREE_DIR_
#define FILE__INDEX__B_PLUS_TREE__B_PLUS_TREE_DIR_

#include "file/index/record.h"

class BPlusTreeParams;
class Page;
class BPlusTree;

class BPlusTreeDir {
friend class BPlusTree;
public:
    BPlusTreeDir(BPlusTreeParams& params, Page& page);
    ~BPlusTreeDir();
    std::unique_ptr<std::pair<Record, int>> insert(Record& record); // returns not null if needs to split
    std::pair<int, int> search_leaf(Record& min);

    bool is_leaf() { return false; }
    int get_count() { return *count; }
    static inline int instance_count = 0;

private:
    BPlusTreeParams& params;
    Page& page;
    int* count;
    uint64_t* records;
    int* dirs;

    int search_dir_index(int from, int to, Record& record);
    void rotate_records(int from, int to);
    void rotate_dirs(int from, int to);
    void update_record(int index, Record& record);
    void update_dir(int index, int dir);
    void split(Record& record);
};

#endif //FILE__INDEX__B_PLUS_TREE__B_PLUS_TREE_DIR_