#include "file/index/bplus_tree/bplus_tree.h"

#include "file/buffer_manager.h"
#include "file/index/record.h"
#include "file/index/bplus_tree/bplus_tree_dir.h"
#include "file/index/bplus_tree/bplus_tree_leaf.h"
#include "file/index/bplus_tree/bplus_tree_params.h"

#include <iostream>
#include <memory>

using namespace std;

BPlusTree::BPlusTree(BPlusTreeParams& params)
    : params(params)
{
    Page& root_page = params.buffer_manager.get_page(0, params.dir_path);
    root = make_unique<BPlusTreeDir>(params, root_page);
}

unique_ptr<BPlusTree::Iter> BPlusTree::get_range(unique_ptr<Record> min, unique_ptr<Record> max)
{
    pair<int, int> page_number_and_pos = root->search_leaf(*min);
    return make_unique<Iter>(params, page_number_and_pos.first, page_number_and_pos.second, std::move(max));
}

void BPlusTree::insert(Record& record)
{
    if (params.is_empty) {
        create_new(record);
        params.is_empty = false;
        return;
    }
    auto split_key = root->insert(record);
}

// Insert first key at root, create leaf
void BPlusTree::create_new(Record& record)
{
    Page& leaf_page = params.buffer_manager.get_page(0, params.leaf_path);
    BPlusTreeLeaf first_leaf = BPlusTreeLeaf(params, leaf_page);
    first_leaf.create_new(record);
}

/******************************* Iter ********************************/
BPlusTree::Iter::Iter(BPlusTreeParams& params, int leaf_page_number, int current_pos, unique_ptr<Record> max)
    : max(move(max)), params(params)
{
    //std::cout << "Creating BPTree iter (page_number: " << leaf_page_number << ", current_pos: " << current_pos << ", max: "<< this->max->ids[0] << "," << this->max->ids[1] << " )\n";
    current_leaf = make_unique<BPlusTreeLeaf>(params, params.buffer_manager.get_page(leaf_page_number, params.leaf_path));
    this->current_pos = current_pos;
}

unique_ptr<Record> BPlusTree::Iter::next()
{
    if (current_pos < current_leaf->get_count()) {
        unique_ptr<Record> res = make_unique<Record>(current_leaf->get_record(current_pos));
        // check if res is less than max
        for (int i = 0; i < params.record_size; i++) {
            if (res->ids[i] < max->ids[i]) {
                current_pos++;
                return res;
            }
            else if (res->ids[i] > max->ids[i]) {
                return nullptr;
            }
            // continue if they were equal
        }
        current_pos++;
        return res; // res == max
    }
    else if (current_leaf->has_next()) {
        // cout << "Iter(" << max->ids[0] << "," << max->ids[1] << ")changed to next leaf (from " << current_leaf->page.page_number;
        current_leaf = current_leaf->next_leaf();
        // cout << ", to " << current_leaf->page.page_number << ")\n";
        current_pos = 0;
        return next();
    }
    else {
        return nullptr;
    }
}
