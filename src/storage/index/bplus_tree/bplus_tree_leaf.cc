#include "bplus_tree_leaf.h"

#include <iostream>
#include <cstring>

#include "storage/buffer_manager.h"
#include "storage/index/bplus_tree/bplus_tree.h"

using namespace std;

template <std::size_t N>
BPlusTreeLeaf<N>::BPlusTreeLeaf(Page& _page) :
    page(_page),
    leaf_file_id(page.page_id.file_id)
{
    value_count = reinterpret_cast<uint32_t*>(page.get_bytes());
    next_leaf   = reinterpret_cast<int*>(page.get_bytes() + sizeof(uint32_t));
    records     = reinterpret_cast<uint64_t*>(page.get_bytes() + (2*sizeof(int)) );
}


template <std::size_t N>
BPlusTreeLeaf<N>::~BPlusTreeLeaf() {
    buffer_manager.unpin(page);
}


// template <std::size_t N>
// unique_ptr<Record<N>> BPlusTreeLeaf<N>::get(const Record<N>& key) {
//     int index = search_index(0, *value_count-1, key);

//     if (equal_record(key, index)) {
//         std::array<uint64_t, N> ids;
//         for (int i = 0; i < N; i++) {
//             ids[i] = records[index * (N + 1) + i];
//         }
//         return make_unique<Record>(ids);
//     }
//     else {
//         return nullptr;
//     }
// }


template <std::size_t N>
unique_ptr<Record<N>> BPlusTreeLeaf<N>::get_record(int pos) {
    std::array<uint64_t, N> ids;
    for (uint_fast32_t i = 0; i < N; i++) {
        ids[i] = records[pos*N + i];
    }
    return make_unique<Record<N>>(ids);
}


template <std::size_t N>
unique_ptr<BPlusTreeLeaf<N>> BPlusTreeLeaf<N>::get_next_leaf() {
    Page& new_page = buffer_manager.get_page(leaf_file_id, *next_leaf);
    return make_unique<BPlusTreeLeaf<N>>(new_page);
}


template <std::size_t N>
unique_ptr<BPlusTreeSplit<N>> BPlusTreeLeaf<N>::insert(const Record<N>& record) {
    uint_fast32_t index = search_index(0, *value_count-1, record);
    if (equal_record(record, index)) {
        for (uint_fast32_t i = 0; i < N; i++) {
            cout << record.ids[i] << " ";
        }
        cout << "\n";
        for (uint_fast32_t i = 0; i < N; i++) {
            cout << records[N*index + i] << " ";
        }
        cout << "\n";

        throw std::logic_error("Inserting duplicated record into BPlusTree.");
    }

    if (*value_count < BPlusTree<N>::leaf_max_records) {
        // shift right from index to *count-1
        shift_right_records(index, *value_count-1);

        for (uint_fast32_t i = 0; i < N; i++) {
            records[index*N + i] = record.ids[i];
        }
        (*value_count)++;
        this->page.make_dirty();
        return nullptr;
    }
    else {
        // poner nuevo record y guardar el ultimo (que no cabe)
        auto last_key = std::array<uint64_t, N>();
        if (index == *value_count) { // la llave a insertar es la ultima
            for (uint_fast32_t i = 0; i < N; i++) {
                last_key[i] = record.ids[i];
            }
        }
        else {
            // guardar ultima llave
            for (uint_fast32_t i = 0; i < N; i++) {
                last_key[i] = records[(*value_count-1)*N + i];
            }

            shift_right_records(index, *value_count-2);
            for (uint_fast32_t i = 0; i < N; i++) {
                records[index*N + i] = record.ids[i];
            }
        }

        // crear nueva hoja
        auto& new_page = buffer_manager.append_page(leaf_file_id);
        auto new_leaf = BPlusTreeLeaf<N>(new_page);

        *new_leaf.next_leaf = *next_leaf;
        *next_leaf = new_leaf.page.get_page_number();

        // write records
        auto middle_index = (*value_count+1)/2;

        std::memcpy(
            new_leaf.records,
            &records[middle_index * N],
            (BPlusTree<N>::leaf_max_records - middle_index) * N * sizeof(uint64_t)
        );

        std::memcpy(
            &new_leaf.records[(BPlusTree<N>::leaf_max_records - middle_index) * N],
            last_key.data(),
            N * sizeof(uint64_t)
        );

        // update counts
        *value_count = middle_index;
        *new_leaf.value_count = (BPlusTree<N>::leaf_max_records/2) + 1;

        // split_key is the first in the new leaf
        std::array<uint64_t, N> split_key;
        for (uint_fast32_t i = 0; i < N; i++) {
            split_key[i] = new_leaf.records[i];
        }
        auto split_record = Record<N>(split_key);
        this->page.make_dirty();
        new_page.make_dirty();

        return make_unique<BPlusTreeSplit<N>>(split_record, new_page.get_page_number());
    }
}


template <std::size_t N>
void BPlusTreeLeaf<N>::create_new(const Record<N>& record) {
    for (uint_fast32_t i = 0; i < N; i++) {
        records[i] = record.ids[i];
    }
    (*value_count)++;
    this->page.make_dirty();
}


template <std::size_t N>
SearchLeafResult BPlusTreeLeaf<N>::search_leaf(const Record<N>& min) {
    auto index = search_index(0, *value_count-1, min);
    return SearchLeafResult(page.get_page_number(), index);
}


// returns the position of the minimum key greater (or equal) than the record given.
template <std::size_t N>
uint_fast32_t BPlusTreeLeaf<N>::search_index(int from, int to, const Record<N>& record) {
    if (from >= to) {
        for (uint_fast32_t i = 0; i < N; i++) {
            auto id = records[from*N + i];
            if (record.ids[i] < id) {
                return from;
            }
            else if (record.ids[i] > id) {
                return from+1;
            }
            // continue if they were equal
        }
        return from;
    }
    auto middle = (from + to) / 2;

    for (uint_fast32_t i = 0; i < N; i++) {
        auto id = records[middle*N + i];
        if (record.ids[i] < id) { // record is smaller
            return search_index(from, middle-1, record);
        }
        else if (record.ids[i] > id) { // record is greater
            return search_index(middle+1, to, record);
        }
        // continue if they were equal
    }
    // record is equal
    return middle;
}


template <std::size_t N>
void BPlusTreeLeaf<N>::shift_right_records(uint_fast32_t from, uint_fast32_t to) {
    for (uint_fast32_t i = to; i >= from; i--) {
        for (uint_fast32_t j = 0; j < N; j++) {
            records[(i+1)*N + j] = records[i*N + j];
        }
    }
}

template <std::size_t N>
bool BPlusTreeLeaf<N>::equal_record(const Record<N>& record, uint_fast32_t index) {
    for (uint_fast32_t i = 0; i < N; i++) {
        if (records[N*index + i] != record.ids[i]) {
            return false;
        }
    }
    return true;
}

template <std::size_t N>
void BPlusTreeLeaf<N>::print() const {
    cout << "Printing Leaf:\n";
    for (uint_fast32_t i = 0; i < *value_count; i++) {
        cout << "  (";
        for (uint_fast32_t j = 0; j < N; j++) {
            if (j != 0)
                cout << ", ";
            cout << records[i*N + j];
        }
        cout << ")\n";
    }
}


template <std::size_t N>
bool BPlusTreeLeaf<N>::check() const {
    if (*value_count <= 0) {
        cerr << "ERROR: value_count should be greater than 0. ";
        cerr << "       got: " << *value_count << "\n";
    }

    if (*value_count > 1) {
        // check keys are ordered
        std::array<uint64_t, N> x;
        std::array<uint64_t, N> y;

        uint_fast32_t current_pos = 0;
        while (current_pos < N) {
            x[current_pos] = records[current_pos];
            current_pos++;
        }

        for (uint_fast32_t k = 1; k < *value_count; k++) {
            for (uint_fast32_t i = 0; i < N; i++) {
                y[i] = records[current_pos++];
            }
            if (y <= x) {
                cerr << "ERROR: bad record order at BPlusTreeLeaf\n";
                print();
                return false;
            }
            x = y;
        }
    }
    return true;
}
