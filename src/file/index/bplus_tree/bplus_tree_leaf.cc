#include "file/page.h"
#include "file/buffer_manager.h"
#include "file/index/record.h"
#include "file/index/bplus_tree/bplus_tree_leaf.h"
#include "file/index/bplus_tree/bplus_tree_params.h"
#include <iostream>

BPlusTreeLeaf::BPlusTreeLeaf(BPlusTreeParams& params, Page& page)
    : params(params), page(page)
{
    count = (int*) &page.bytes[0];
    next = (int*) &page.bytes[sizeof(int)];
    records = (uint64_t*) &page.bytes[2*sizeof(int)];
    instance_count++;

    //std::cout << "Creating Leaf("<<page.page_number<<"), count: " << *count << "\n";
}

BPlusTreeLeaf::~BPlusTreeLeaf()
{
    instance_count--;
}

Record BPlusTreeLeaf::get_record(int pos)
{
    std::unique_ptr<uint64_t[]> ids = std::make_unique<uint64_t[]>(params.record_size);
    for (int i = 0; i < params.record_size; i++) {
        ids[i] = records[pos*params.record_size + i];
    }
    return Record(ids.get(), params.record_size);
}

std::unique_ptr<BPlusTreeLeaf> BPlusTreeLeaf::next_leaf()
{
    Page& new_page = params.buffer_manager.get_page(*next, params.leaf_path);
    return std::make_unique<BPlusTreeLeaf>(params, new_page);
}

std::unique_ptr<std::pair<Record, int>> BPlusTreeLeaf::insert(Record& record) // TODO: make unique pointer
{
    int index = search_index(0, *count-1, record);

    if (*count < params.leaf_max_records) {
        // rotate right from index to *count-1
        for (int i = (*count-1); i >= index; i--) {
            for (int j = 0; j < params.record_size; j++) {
                records[(i+1)*params.record_size + j] = records[i*params.record_size + j];
            }
        }

        for (int i = 0; i < params.record_size; i++) {
            records[index*params.record_size + i] = record.ids[i];
        }
        (*count)++;
        return nullptr;
    }
    else {
        std::cout << "Splitting leaf\n";
        // poner nuevo record y guardar el ultimo (que no cabe)
        std::unique_ptr<uint64_t[]> last_key = std::make_unique<uint64_t[]>(params.record_size);
        if (index == *count) { // la llave a insertar es la ultima
            for (int i = 0; i < params.record_size; i++) {
                last_key[i] = record.ids[i];
            }
        }
        else {
            // guardar ultima llave
            for (int i = 0; i < params.record_size; i++) {
                last_key[i] = records[(*count-1)*params.record_size+i];
            }

            for (int i = (*count-2); i >= index; i--) {
                for (int j = 0; j < params.record_size; j++) {
                    records[(i+1)*params.record_size + j] = records[i*params.record_size + j];
                }
            }
            for (int i = 0; i < params.record_size; i++) {
                records[index*params.record_size + i] = record.ids[i];
            }
        }

        // crear nueva hoja
        Page& new_page = params.buffer_manager.append_page(params.leaf_path);
        BPlusTreeLeaf new_leaf = BPlusTreeLeaf(params, new_page);

        *new_leaf.next = *next;
        *next = new_leaf.page.page_number;

        // write records
        int middle_index = (*count+1)/2;
        int last_index = params.leaf_max_records * params.record_size;
        int new_leaf_pos = 0;
        for (int i = middle_index * params.record_size; i < last_index; i++, new_leaf_pos++) {
            new_leaf.records[new_leaf_pos] = records[i];
        }
        // write last record
        for (int i = 0; i < params.record_size; i++, new_leaf_pos++) {
            new_leaf.records[new_leaf_pos] = last_key[i];
        }

        // update counts
        *count = middle_index;
        *new_leaf.count = (params.leaf_max_records/2) + 1;

        // split_key is the first in the new leaf
        std::unique_ptr<uint64_t[]> split_key = std::make_unique<uint64_t[]>(params.record_size);
        for (int i = 0; i < params.record_size; i++) {
            split_key[i] = new_leaf.records[i];
        }
        Record split_record = Record(split_key.get(), params.record_size);
        return std::make_unique<std::pair<Record, int>>(split_record, new_page.page_number);
    }
}

void BPlusTreeLeaf::create_new(Record& record)
{
    for (int i = 0; i < params.record_size; i++) {
        records[i] = record.ids[i];
    }
    (*count)++;
}

std::pair<int, int> BPlusTreeLeaf::search_leaf(Record& min)
{
    int index = search_index(0, *count-1, min);
    /*std::cout << "BPlusTreeLeaf::search_leaf\n";
    for (int i = 0; i < (*count)*2; i++) {
        std::cout << "   " << records[i++] << ", " << records[i] << "\n";
    }
    std::cout << "found " << index << "\n";*/
    return std::pair<int, int>(page.page_number, index);
}


// returns the position of the minimum key greater (or equal) than the record given.
int BPlusTreeLeaf::search_index(int from, int to, Record& record)
{
    //std::cout << "search index(" << from << ", " << to << ") count: " << *count << " \n";
    if (from >= to) {
        bool record_greater = false; // if records is greater or equal than from

        for (int i = 0; i < params.record_size; i++) {
            auto id = records[from*params.record_size + i];
            if (record.ids[i] < id) {
                break;
            }
            else if (record.ids[i] > id) {
                record_greater = true;
                break;
            }
            // continue if they were equal
        }
        if (record_greater) {
            return from+1;
        }
        else {
            return from;
        }
    }
    int middle = (from + to) / 2;
    bool record_is_greater = true; // if records is greater or equal than middle

    for (int i = 0; i < params.record_size; i++) {
        auto id = records[middle*params.record_size + i];
        if (record.ids[i] < id) {
            record_is_greater = false;
            goto not_equal;
        }
        else if (record.ids[i] > id) {
            goto not_equal;
        }
        // continue if they were equal
    }
    return search_index(middle, middle, record);
not_equal:
    if (record_is_greater) {
        return search_index(middle+1, to, record);
    }
    else {
        return search_index(from, middle-1, record);
    }
}