#include "file/page.h"
#include "file/buffer_manager.h"
#include "file/index/record.h"
#include "file/index/bplus_tree/bplus_tree_leaf.h"
#include "file/index/bplus_tree/bplus_tree_params.h"
#include <iostream>

BPlusTreeLeaf::BPlusTreeLeaf(const BPlusTreeParams& params, Page& page)
    : params(params), page(page)
{
    count = (int*) &page.get_bytes()[0];
    next = (int*) &page.get_bytes()[sizeof(int)];
    records = (uint64_t*) &page.get_bytes()[2*sizeof(int)];
    // std::cout << "count  : " << (void *)count << "\n";
    // std::cout << "next   : " << (void *)next << "\n";
    // std::cout << "records: " << (void *)records << "\n";
}

BPlusTreeLeaf::~BPlusTreeLeaf()
{
    page.unpin();
}


void BPlusTreeLeaf::edit(const Record& key, const Record& value)
{
    int index = search_index(0, *count-1, key);

    if (equal_record(key, index)) {
        for (int i = 0; i < params.value_size; i++) {
            records[params.total_size*index + params.key_size + i] = value.ids[i];
        }
    }
    else {
        throw std::logic_error("Editing record threw error: key not found.");
    }
}


std::unique_ptr<Record> BPlusTreeLeaf::get(const Record& key)
{
    int index = search_index(0, *count-1, key);

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


std::unique_ptr<Record> BPlusTreeLeaf::get_record(int pos)
{
    std::vector<uint64_t> ids(params.total_size);
    for (int i = 0; i < params.total_size; i++) {
        ids[i] = records[pos*params.total_size + i];
    }
    return std::make_unique<Record>(ids);
}

std::unique_ptr<BPlusTreeLeaf> BPlusTreeLeaf::next_leaf()
{
    Page& new_page = BufferManager::get_page(*next, params.leaf_path);
    return std::make_unique<BPlusTreeLeaf>(params, new_page);
}


std::unique_ptr<std::pair<Record, int>> BPlusTreeLeaf::insert(const Record& key, const Record& value)
{
    int index = search_index(0, *count-1, key);
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

    if (*count < params.leaf_max_records) {
        // shift right from index to *count-1
        shift_right_records(index, *count-1);

        for (int i = 0; i < params.key_size; i++) {
            records[index*params.total_size + i] = key.ids[i];
        }
        for (int i = 0; i < params.value_size; i++) {
            records[index*params.total_size + params.key_size + i] = value.ids[i];
        }
        (*count)++;
        this->page.make_dirty();
        return nullptr;
    }
    else {
        // poner nuevo record y guardar el ultimo (que no cabe)
        std::unique_ptr<uint64_t[]> last_key = std::make_unique<uint64_t[]>(params.total_size);
        if (index == *count) { // la llave a insertar es la ultima
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
                last_key[i] = records[(*count-1)*params.total_size+i];
            }

            shift_right_records(index, *count-2);
            for (int i = 0; i < params.key_size; i++) {
                records[index*params.total_size + i] = key.ids[i];
            }
            for (int i = 0; i < params.value_size; i++) {
                records[index*params.total_size + params.key_size + i] = value.ids[i];
            }
        }

        // crear nueva hoja
        Page& new_page = BufferManager::append_page(params.leaf_path);
        BPlusTreeLeaf new_leaf = BPlusTreeLeaf(params, new_page);

        *new_leaf.next = *next;
        *next = new_leaf.page.get_page_number();

        // write records
        int middle_index = (*count+1)/2;
        int last_index = params.leaf_max_records * params.total_size;
        int new_leaf_pos = 0;
        for (int i = middle_index * params.total_size; i < last_index; i++, new_leaf_pos++) {
            new_leaf.records[new_leaf_pos] = records[i];
        }
        // write last record
        for (int i = 0; i < params.total_size; i++, new_leaf_pos++) {
            new_leaf.records[new_leaf_pos] = last_key[i];
        }

        // update counts
        *count = middle_index;
        *new_leaf.count = (params.leaf_max_records/2) + 1;

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

void BPlusTreeLeaf::create_new(const Record& key, const Record& value)
{
    for (int i = 0; i < params.key_size; i++) {
        records[i] = key.ids[i];
    }
    for (int i = 0; i < params.value_size; i++) {
        records[params.key_size+i] = value.ids[i];
    }
    (*count)++;
    this->page.make_dirty();
}

std::pair<int, int> BPlusTreeLeaf::search_leaf(const Record& min)
{
    int index = search_index(0, *count-1, min);
    return std::pair<int, int>(page.get_page_number(), index);
}


// returns the position of the minimum key greater (or equal) than the record given.
int BPlusTreeLeaf::search_index(int from, int to, const Record& record)
{
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


void BPlusTreeLeaf::shift_right_records(int from, int to)
{
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
    for (int i = 0; i < *count; i++) {
        std::cout << "  (";
        for (int j = 0; j < params.total_size; j++) {
            if (j != 0)
                std::cout << ", ";
            std::cout << records[i*params.total_size+j];
        }
        std::cout << ")\n";
    }
}