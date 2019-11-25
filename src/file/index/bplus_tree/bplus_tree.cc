#include "file/index/bplus_tree/bplus_tree.h"

#include "file/buffer_manager.h"
#include "file/index/record.h"
#include "file/index/ordered_file/ordered_file.h"
#include "file/index/bplus_tree/bplus_tree_dir.h"
#include "file/index/bplus_tree/bplus_tree_leaf.h"
#include "file/index/bplus_tree/bplus_tree_params.h"

#include <iostream>
#include <memory>

using namespace std;

BPlusTree::BPlusTree(const BPlusTreeParams& params)
    : params(params)
{
    root = make_unique<BPlusTreeDir>(params, BufferManager::get_page(0, params.dir_file_id));
    BPlusTreeLeaf first_leaf(params, BufferManager::get_page(0, params.leaf_file_id)); // just to see if BPT is empty
    is_empty = *first_leaf.count == 0;
}


void BPlusTree::bulk_import(OrderedFile& ordered_file)
{
    ordered_file.begin_iter();
    // first leaf
    BPlusTreeLeaf first_leaf(params, BufferManager::get_page(0, params.leaf_file_id));
    *first_leaf.count = ordered_file.next_tuples(first_leaf.records, params.leaf_max_records);
    // root.dirs[0] = 0;
    // cout << *first_leaf.count << "\n";
    if (ordered_file.has_more_tuples()) {
        *first_leaf.next = 1;
    }

    while (ordered_file.has_more_tuples()) {

        BPlusTreeLeaf new_leaf(params, BufferManager::append_page(params.leaf_file_id));
        *new_leaf.count = ordered_file.next_tuples(new_leaf.records, params.leaf_max_records);
        // cout << *new_leaf.count << "\n";

        if (ordered_file.has_more_tuples()) {
            *new_leaf.next = new_leaf.page.get_page_number() + 1;
        }
        root->bulk_insert(new_leaf);
        new_leaf.page.make_dirty();
    }
}


unique_ptr<BPlusTree::Iter> BPlusTree::get_range(const Record& min, const Record& max)
{
    pair<int, int> page_number_and_pos = root->search_leaf(min);
    return make_unique<Iter>(params, page_number_and_pos.first, page_number_and_pos.second, max);
}


void BPlusTree::insert(const Record& record)
{
    if (is_empty) {
        create_new(record, Record::get_empty_record());
        is_empty = false;
        return;
    }
    root->insert(record, Record::get_empty_record());
}

void BPlusTree::insert(const Record& key, const Record& value)
{
    if (is_empty) {
        create_new(key, value);
        is_empty = false;
        return;
    }
    root->insert(key, value);
}

// Insert first key at root, create leaf
void BPlusTree::create_new(const Record& key, const Record& value)
{
    Page& leaf_page = BufferManager::get_page(0, params.leaf_file_id);
    BPlusTreeLeaf first_leaf = BPlusTreeLeaf(params, leaf_page);
    first_leaf.create_new(key, value);
}

void BPlusTree::edit(const Record& key, const Record& value)
{
    root->edit(key, value);
}

unique_ptr<Record> BPlusTree::get(const Record& key)
{
    return root->get(key);
}

/******************************* Iter ********************************/
BPlusTree::Iter::Iter(const BPlusTreeParams& params, int leaf_page_number, int current_pos, const Record& max)
    : params(params), max(max)
{
    current_leaf = make_unique<BPlusTreeLeaf>(params, BufferManager::get_page(leaf_page_number, params.leaf_file_id));
    this->current_pos = current_pos;
}

unique_ptr<Record> BPlusTree::Iter::next()
{
    if (current_pos < current_leaf->get_count()) {
        unique_ptr<Record> res = current_leaf->get_record(current_pos);
        // check if res is less than max
        for (int i = 0; i < params.key_size; i++) {
            if (res->ids[i] < max.ids[i]) {
                current_pos++;
                return res;
            }
            else if (res->ids[i] > max.ids[i]) {
                return nullptr;
            }
            // continue if they were equal
        }
        current_pos++;
        return res; // res == max
    }
    else if (current_leaf->has_next()) {
        current_leaf = current_leaf->next_leaf();
        current_pos = 0;
        return next();
    }
    else {
        return nullptr;
    }
}
