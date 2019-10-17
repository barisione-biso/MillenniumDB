#ifndef FILE__INDEX__B_PLUS_TREE_H_
#define FILE__INDEX__B_PLUS_TREE_H_

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
    BPlusTree(const BPlusTreeParams& params);
    ~BPlusTree() = default;

    void insert(const Record& record);
    void insert(const Record& key, const Record& value);

    void edit(const Record& key, const Record& value);
    unique_ptr<Record> get(const Record& record);

    const BPlusTreeParams& params;

    class Iter {
        public:
            Iter(const BPlusTreeParams& params, int leaf_page_number, int current_pos, const Record& max);
            ~Iter() = default;
            unique_ptr<Record> next();

        private:
            int current_pos;
            const BPlusTreeParams& params;
            const Record max;
            unique_ptr<BPlusTreeLeaf> current_leaf;
    };

    unique_ptr<BPlusTree::Iter> get_range(const Record& min, const Record& max);

private:
    bool is_empty;
    unique_ptr<BPlusTreeDir> root;
    void create_new(const Record& record);
    void create_new(const Record& key, const Record& value);
};

#endif //FILE__INDEX__B_PLUS_TREE_H_
