#ifndef FILE__INDEX__B_PLUS_TREE__B_PLUS_TREE_
#define FILE__INDEX__B_PLUS_TREE__B_PLUS_TREE_

#include <string>
#include <memory>

#include "file/index/bplus_tree/bplus_tree_dir.h"
#include "file/index/bplus_tree/bplus_tree_leaf.h"

class Record;
class BPlusTreeParams;

using namespace std;

class BPlusTree
{
public:
    BPlusTree(BPlusTreeParams& params);
    ~BPlusTree() = default;

    void insert(Record& record);
    //void remove(Record& record);
    BPlusTreeParams& params; // private?

    class Iter {
        public:
            Iter(BPlusTreeParams& params, int leaf_page_number, int current_pos, unique_ptr<Record> max);
            ~Iter() = default;
            unique_ptr<Record> next();

        private:
            unique_ptr<BPlusTreeLeaf> current_leaf;
            int current_pos;
            unique_ptr<Record> max;
            BPlusTreeParams& params;
    };

    bool has_record(const Record&);
    unique_ptr<BPlusTree::Iter> get_range(unique_ptr<Record> min, unique_ptr<Record> max);

private:
    unique_ptr<BPlusTreeDir> root; // value?
    void create_new(Record& record);
};

#endif //FILE__INDEX__B_PLUS_TREE__B_PLUS_TREE_
