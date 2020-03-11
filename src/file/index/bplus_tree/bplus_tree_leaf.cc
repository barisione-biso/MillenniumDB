#include "file/page.h"
#include "file/buffer_manager.h"
#include "file/index/record.h"
#include "file/index/bplus_tree/bplus_tree_leaf.h"
#include "file/index/bplus_tree/bplus_tree_params.h"

#include <iostream>
#include <cstring>

using namespace std;

BPlusTreeLeaf::BPlusTreeLeaf(const BPlusTreeParams& params, Page& page)
    : params(params), page(page)
{
    value_count = (int*) page.get_bytes();
    next_leaf   = (int*) (page.get_bytes() + sizeof(int));
    records     = (uint64_t*) (page.get_bytes() + (2*sizeof(int)) );
}


BPlusTreeLeaf::~BPlusTreeLeaf() {
    page.unpin();
}


std::unique_ptr<Record> BPlusTreeLeaf::get(const Record& key) {
    int index = search_index(0, *value_count-1, key);

    if (equal_record(key, index)) {
        std::vector<uint64_t> ids(params.value_size);
        for (int i = 0; i < params.value_size; i++) {
            ids[i] = records[index*params.total_size + params.key_size + i];
        }
        return std::make_unique<Record>(ids);
    }
    else {
        return nullptr;
    }
}


std::unique_ptr<Record> BPlusTreeLeaf::get_record(int pos) {
    std::vector<uint64_t> ids(params.total_size);
    for (int i = 0; i < params.total_size; i++) {
        ids[i] = records[pos*params.total_size + i];
    }
    return std::make_unique<Record>(ids);
}


std::unique_ptr<BPlusTreeLeaf> BPlusTreeLeaf::get_next_leaf() {
    Page& new_page = buffer_manager.get_page(*next_leaf, params.leaf_file_id);
    return std::make_unique<BPlusTreeLeaf>(params, new_page);
}


std::unique_ptr<std::pair<Record, int>> BPlusTreeLeaf::insert(const Record& key, const Record& value) {
    int index = search_index(0, *value_count-1, key);
    if (equal_record(key, index)) {
        for (int i = 0; i < params.key_size; i++) {
            std::cout << key.ids[i] << " ";
        }
        std::cout << "\n";
        for (int i = 0; i < params.key_size; i++) {
            std::cout << records[params.total_size*index + i] << " ";
        }
        std::cout << "\n";

        throw std::logic_error("Inserting key duplicated into BPlusTree.");
    }

    if (*value_count < params.leaf_max_records) {
        // shift right from index to *count-1
        shift_right_records(index, *value_count-1);

        for (int i = 0; i < params.key_size; i++) {
            records[index*params.total_size + i] = key.ids[i];
        }
        for (int i = 0; i < params.value_size; i++) {
            records[index*params.total_size + params.key_size + i] = value.ids[i];
        }
        (*value_count)++;
        this->page.make_dirty();
        return nullptr;
    }
    else {
        // poner nuevo record y guardar el ultimo (que no cabe)
        auto last_key = vector<uint64_t>(params.total_size);
        if (index == *value_count) { // la llave a insertar es la ultima
            for (int i = 0; i < params.key_size; i++) {
                last_key[i] = key.ids[i];
            }
            for (int i = 0; i < params.value_size; i++) {
                last_key[params.key_size + i] = value.ids[i];
            }
        }
        else {
            // guardar ultima llave
            for (int i = 0; i < params.total_size; i++) {
                last_key[i] = records[(*value_count-1)*params.total_size+i];
            }

            shift_right_records(index, *value_count-2);
            for (int i = 0; i < params.key_size; i++) {
                records[index*params.total_size + i] = key.ids[i];
            }
            for (int i = 0; i < params.value_size; i++) {
                records[index*params.total_size + params.key_size + i] = value.ids[i];
            }
        }

        // crear nueva hoja
        auto& new_page = buffer_manager.append_page(params.leaf_file_id);
        auto new_leaf = BPlusTreeLeaf(params, new_page);

        *new_leaf.next_leaf = *next_leaf;
        *next_leaf = new_leaf.page.get_page_number();

        // write records
        int middle_index = (*value_count+1)/2;

        std::memcpy(
            new_leaf.records,
            &records[middle_index * params.total_size],
            (params.leaf_max_records - middle_index) * params.total_size * sizeof(uint64_t)
        );

        std::memcpy(
            &new_leaf.records[(params.leaf_max_records - middle_index) * params.total_size],
            last_key.data(),
            params.total_size * sizeof(uint64_t)
        );

        // update counts
        *value_count = middle_index;
        *new_leaf.value_count = (params.leaf_max_records/2) + 1;

        // split_key is the first in the new leaf
        std::vector<uint64_t> split_key(params.key_size);
        for (int i = 0; i < params.key_size; i++) {
            split_key[i] = (new_leaf.records[i]);
        }
        Record split_record = Record(split_key);
        this->page.make_dirty();
        new_page.make_dirty();

        return std::make_unique<std::pair<Record, int>>(split_record, new_page.get_page_number());
    }
}


void BPlusTreeLeaf::create_new(const Record& key, const Record& value) {
    for (int i = 0; i < params.key_size; i++) {
        records[i] = key.ids[i];
    }
    for (int i = 0; i < params.value_size; i++) {
        records[params.key_size+i] = value.ids[i];
    }
    (*value_count)++;
    this->page.make_dirty();
}


std::pair<int, int> BPlusTreeLeaf::search_leaf(const Record& min) {
    int index = search_index(0, *value_count-1, min);
    return std::pair<int, int>(page.get_page_number(), index);
}


// returns the position of the minimum key greater (or equal) than the record given.
int BPlusTreeLeaf::search_index(int from, int to, const Record& record) {
    if (from >= to) {
        for (int i = 0; i < params.key_size; i++) {
            auto id = records[from*params.total_size + i];
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
    int middle = (from + to) / 2;

    for (int i = 0; i < params.key_size; i++) {
        auto id = records[middle*params.total_size + i];
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


void BPlusTreeLeaf::shift_right_records(int from, int to) {
    for (int i = to; i >= from; i--) {
        for (int j = 0; j < params.total_size; j++) {
            records[(i+1)*params.total_size + j] = records[i*params.total_size + j];
        }
    }
}

bool BPlusTreeLeaf::equal_record(const Record& record, int index) {
    for (int i = 0; i < params.key_size; i++) {
        if (records[params.total_size*index + i] != record.ids[i]) {
            return false;
        }
    }
    return true;
}

void BPlusTreeLeaf::print() const {
    std::cout << "Printing Leaf:\n";
    for (int i = 0; i < *value_count; i++) {
        std::cout << "  (";
        for (int j = 0; j < params.total_size; j++) {
            if (j != 0)
                std::cout << ", ";
            std::cout << records[i*params.total_size+j];
        }
        std::cout << ")\n";
    }
}


bool BPlusTreeLeaf::check() const {
    if (*value_count <= 0) {
        std::cout << "value_count should be greater than 0. ";
        std::cout << "got: " << *value_count << "\n";
    }

    if (*value_count > 1) {
        // check keys are ordered
        auto x = Record(vector<uint64_t>(params.total_size));
        auto y = Record(vector<uint64_t>(params.total_size));

        int current_pos = 0;
        while (current_pos < params.total_size) {
            x.ids[current_pos] = records[current_pos];
            current_pos++;
        }

        for (int k = 1; k < *value_count; k++) {
            for (int i = 0; i < params.total_size; i++) {
                y.ids[i] = records[current_pos++];
            }
            if (y <= x) {
                std::cout << "bad record order at BPlusTreeLeaf\n";
                // std::cout << x.ids[0] << "," << x.ids[1] << "," << x.ids[2]  << "\n"; // TODO:
                // std::cout << y.ids[0] << "," << y.ids[1] << "," << y.ids[2]  << "\n"; // TODO:
                print();
                return false;
            }
            x = y;
        }
    }
    return true;
}
