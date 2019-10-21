#ifndef FILE__INDEX__B_PLUS_TREE_DIR_H_
#define FILE__INDEX__B_PLUS_TREE_DIR_H_

#include "file/index/record.h"

class BPlusTreeParams;
class Page;
class BPlusTree;

class BPlusTreeDir {
friend class BPlusTree;
public:
    BPlusTreeDir(const BPlusTreeParams& params, Page& page);
    ~BPlusTreeDir();

    std::unique_ptr<std::pair<Record, int>> insert(const Record& record); // returns not null if needs to split
    std::unique_ptr<std::pair<Record, int>> insert(const Record& key, const Record& value); // returns not null if needs to split

    void edit(const Record& key, const Record& value);
    std::unique_ptr<Record> get(const Record& key);

    std::pair<int, int> search_leaf(const Record& min);

    bool is_leaf()  { return false; }
    int get_count() { return *count; }

private:
    const BPlusTreeParams& params;
    Page& page;
    int* count;
    uint64_t* records;
    int* dirs;

    int search_dir_index(int from, int to, const Record& record);
    void shift_right_records(int from, int to);
    void shift_right_dirs(int from, int to);
    void update_record(int index, const Record& record);
    void update_dir(int index, int dir);
    void split(const Record& record);
};

#endif //FILE__INDEX__B_PLUS_TREE_DIR_H_