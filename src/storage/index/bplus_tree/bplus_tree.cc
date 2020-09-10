#include "storage/index/bplus_tree/bplus_tree.h"

#include "storage/file_manager.h"
#include "storage/buffer_manager.h"
#include "storage/index/record.h"
#include "storage/index/ordered_file/ordered_file.h"
#include "storage/index/ordered_file/bpt_leaf_provider.h"
#include "storage/index/bplus_tree/bplus_tree_dir.h"
#include "storage/index/bplus_tree/bplus_tree_leaf.h"

#include <iostream>
#include <memory>

using namespace std;

template <std::size_t N>
BPlusTree<N>::BPlusTree(const std::string& name) :
    dir_file_id(file_manager.get_file_id(name + ".dir")),
    leaf_file_id(file_manager.get_file_id(name + ".leaf"))
{
    root = make_unique<BPlusTreeDir<N>>(leaf_file_id, buffer_manager.get_page(dir_file_id, 0));

    // first leaf is readed just to see if BPT is empty
    auto first_leaf = BPlusTreeLeaf<N>(buffer_manager.get_page(leaf_file_id, 0));
    is_empty = first_leaf.value_count == 0;
}


template <std::size_t N>
void BPlusTree<N>::bulk_import(BptLeafProvider& leaf_provider) {
    leaf_provider.begin();

    if (is_empty) {
        // first leaf
        auto first_leaf = BPlusTreeLeaf<N>(buffer_manager.get_page(leaf_file_id, 0));
        first_leaf.value_count = leaf_provider.next_tuples(first_leaf.records, leaf_max_records);
        // root.dirs[0] = 0;
        if (leaf_provider.has_more_tuples()) {
            first_leaf.next_leaf = first_leaf.page.get_page_number() + 1;
        }
        first_leaf.page.make_dirty();
        is_empty = false;
    } else {
        // fill last page
        auto last_leaf = BPlusTreeLeaf<N>(buffer_manager.get_last_page(leaf_file_id));
        if (last_leaf.value_count < leaf_max_records) {
            auto new_tuples = leaf_provider.next_tuples(&last_leaf.records[last_leaf.value_count * N],
                                                        leaf_max_records - last_leaf.value_count);
            last_leaf.value_count += new_tuples;
        }
        if (leaf_provider.has_more_tuples()) {
            last_leaf.next_leaf = last_leaf.page.get_page_number() + 1;
        }
        last_leaf.page.make_dirty();
    }

    while (leaf_provider.has_more_tuples()) {
        auto new_leaf = BPlusTreeLeaf<N>(buffer_manager.append_page(leaf_file_id));
        new_leaf.value_count = leaf_provider.next_tuples(new_leaf.records, leaf_max_records);

        if (new_leaf.value_count <= 0) {
            cout << "Wrong new_leaf.value_count: " << new_leaf.value_count << "\n";
        }

        if (leaf_provider.has_more_tuples()) {
            new_leaf.next_leaf = new_leaf.page.get_page_number() + 1;
        }
        root->bulk_insert(new_leaf);
        new_leaf.page.make_dirty();
    }
}


template <std::size_t N>
unique_ptr<BptIter<N>> BPlusTree<N>::get_range(const Record<N>& min, const Record<N>& max) {
    auto page_number_and_pos = root->search_leaf(min);
    return make_unique<BptIter<N>>(leaf_file_id, page_number_and_pos.page_number, page_number_and_pos.result_index, max);
}


template <std::size_t N>
void BPlusTree<N>::insert(const Record<N>& record) {
    if (is_empty) {
        create_new(record);
        is_empty = false;
        return;
    }
    root->insert(record);
}


// Insert first key at root, create leaf
template <std::size_t N>
void BPlusTree<N>::create_new(const Record<N>& record) {
    auto& leaf_page = buffer_manager.get_page(leaf_file_id, 0);
    auto first_leaf = BPlusTreeLeaf<N>(leaf_page);
    first_leaf.create_new(record);
}


// template <std::size_t N>
// unique_ptr<Record<N>> BPlusTree<N>::get(const Record<N>& key) {
//     return root->get(key);
// }


template <std::size_t N>
bool BPlusTree<N>::check() const {
    return root->check();
}


/******************************* BptIter ********************************/
template <std::size_t N>
BptIter<N>::BptIter(FileId leaf_file_id, int leaf_page_number, int current_pos, const Record<N>& max) :
    leaf_file_id(leaf_file_id),
    max(max)
{
    current_leaf = make_unique<BPlusTreeLeaf<N>>(buffer_manager.get_page(leaf_file_id, leaf_page_number));
    this->current_pos = current_pos;
}


template <std::size_t N>
unique_ptr<Record<N>> BptIter<N>::next() {
    if (current_pos < current_leaf->get_value_count()) {
        unique_ptr<Record<N>> res = current_leaf->get_record(current_pos);
        // check if res is less than max
        for (unsigned int i = 0; i < N; ++i) {
            if (res->ids[i] < max.ids[i]) {
                ++current_pos;
                return res;
            }
            else if (res->ids[i] > max.ids[i]) {
                return nullptr;
            }
            // continue if they were equal
        }
        ++current_pos;
        return res; // res == max
    }
    else if (current_leaf->has_next()) {
        current_leaf = current_leaf->get_next_leaf();
        current_pos = 0;
        return next();
    }
    else {
        return nullptr;
    }
}
