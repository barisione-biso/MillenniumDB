#include <iostream>
#include <utility>

#include "file/page.h"
#include "file/buffer_manager.h"
#include "file/index/record.h"
#include "file/index/bplus_tree/bplus_tree.h"
#include "file/index/bplus_tree/bplus_tree_dir.h"
#include "file/index/bplus_tree/bplus_tree_leaf.h"
#include "file/index/bplus_tree/bplus_tree_params.h"

BPlusTreeDir::BPlusTreeDir(BPlusTreeParams& params, Page& page)
    : params(params), page(page)
{
    count = (int*) &page.bytes[0];
    records = (uint64_t*) &page.bytes[sizeof(int)];
    dirs = (int*) &page.bytes[sizeof(int)+((sizeof(uint64_t)*params.dir_max_records*params.record_size))];
    instance_count++;
}

BPlusTreeDir::~BPlusTreeDir()
{
    instance_count--;
}

std::unique_ptr<std::pair<Record, int>> BPlusTreeDir::insert(Record& record)
{
    int index = 0;
    if (*count > 0) index = search_dir_index(0, *count, record);
    int page_pointer = dirs[index];
    std::unique_ptr<std::pair<Record, int>> split_record_index;

    if (page_pointer < 0) { // negative number: pointer to dir
        Page& child_page = params.buffer_manager.get_page(page_pointer*-1, params.dir_path);
        BPlusTreeDir child =  BPlusTreeDir(params, child_page);
        split_record_index = child.insert(record);
    }
    else { // positive number: pointer to leaf
        Page& child_page = params.buffer_manager.get_page(page_pointer, params.leaf_path);
        BPlusTreeLeaf child =  BPlusTreeLeaf(params, child_page);
        split_record_index = child.insert(record);
    }

    if (split_record_index != nullptr) {
        std::cout << "Splited received ("<< (int) split_record_index->first.ids[0] << ", " << (int) split_record_index->first.ids[1] << ")\n";

        int splitted_index = search_dir_index(0, *count, split_record_index->first);
        std::cout << "splitted_index = " << splitted_index << "\n";
        // Case 1: no need to split this node
        if (*count < params.dir_max_records){
            rotate_records(splitted_index, *count-1);
            rotate_dirs(splitted_index+1, *count);
            update_record(splitted_index, split_record_index->first);
            update_dir(splitted_index+1, split_record_index->second);
            (*count)++;
            return nullptr;
        }
        // Case 2: we need to split this node and this node is the root
        else if (page.page_number == 0) {
            std::cout << "splitting root\n";

            // poner nuevo record/dir y guardar el ultimo (que no cabe)
            std::unique_ptr<uint64_t[]> last_key = std::make_unique<uint64_t[]>(params.record_size);
            int last_dir;
            if (splitted_index == *count) { // splitted key is the last key
                for (int i = 0; i < params.record_size; i++) {
                    last_key[i] = split_record_index->first.ids[i];
                }
                last_dir = split_record_index->second;
            }
            else {
                for (int i = 0; i < params.record_size; i++) {
                    last_key[i] = records[(*count-1)*params.record_size+i];
                }
                last_dir = dirs[*count];
                rotate_records(splitted_index, *count-2);
                rotate_dirs(splitted_index+1, *count-1);
                update_record(splitted_index, split_record_index->first);
                update_dir(splitted_index+1, split_record_index->second);
            }
            int middle_index = (*count+1)/2;
            Page& new_left_page = params.buffer_manager.append_page(params.dir_path);
            Page& new_right_page = params.buffer_manager.append_page(params.dir_path);

            BPlusTreeDir new_left_dir = BPlusTreeDir(params, new_left_page);
            BPlusTreeDir new_right_dir = BPlusTreeDir(params, new_right_page);

            // write left records from 0 to (middle_index-1)
            for (int i = 0; i < middle_index * params.record_size; i++) {
                new_left_dir.records[i] = records[i];
            }
            // write right records from (middle_index+1) to (*count-1) plus the last record saved before
            int right_record_pos = 0;
            for (int i = (middle_index+1)*params.record_size; i < params.dir_max_records*params.record_size; i++, right_record_pos++) {
                new_right_dir.records[right_record_pos] = records[i];
            }
            for (int i = 0; i < params.record_size; i++, right_record_pos++) {
                new_right_dir.records[right_record_pos] = last_key[i];
            }
            // write left dirs from 0 to middle_index
            for (int i = 0; i <= middle_index; i++) {
                new_left_dir.dirs[i] = dirs[i];
            }
            // write right dirs from middle_index + 1 to *count plus the last dir saved before
            int right_dir_pos = 0;
            for (int i = middle_index + 1; i <= *count; i++, right_dir_pos++) {
                new_right_dir.dirs[right_dir_pos] = dirs[i];
            }
            new_right_dir.dirs[right_dir_pos] = last_dir;
            // update counts
            *count = 1;
            *new_left_dir.count = middle_index;
            *new_right_dir.count = params.dir_max_records - middle_index;

            // record at middle_index becomes the first and only record of the root
            for (int i = 0; i < params.record_size; i++) {
                records[i] = records[middle_index*params.record_size + i];
            }
            dirs[0] = new_left_dir.page.page_number * -1;
            dirs[1] = new_right_dir.page.page_number * -1;
            return nullptr;
        }
        else { // normal split split
            std::cout << "splitting dir\n";

            // poner nuevo record/dir y guardar el ultimo (que no cabe)
            std::unique_ptr<uint64_t[]> last_key = std::make_unique<uint64_t[]>(params.record_size);
            int last_dir;
            if (splitted_index == *count) { // splitted key is the last key
                for (int i = 0; i < params.record_size; i++) {
                    last_key[i] = split_record_index->first.ids[i];
                }
                last_dir = split_record_index->second;
            }
            else {
                for (int i = 0; i < params.record_size; i++) {
                    last_key[i] = records[(*count-1)*params.record_size+i];
                }
                last_dir = dirs[*count];
                rotate_records(splitted_index, *count-2);
                rotate_dirs(splitted_index+1, *count-1);
                update_record(splitted_index, split_record_index->first);
                update_dir(splitted_index+1, split_record_index->second);
            }
            int middle_index = (*count+1)/2;

            Page& new_page = params.buffer_manager.append_page(params.dir_path);
            BPlusTreeDir new_dir = BPlusTreeDir(params, new_page);

            // write records from (middle_index+1) to (*count-1) plus the last record saved before
            int new_record_pos = 0;
            for (int i = (middle_index+1)*params.record_size; i < params.dir_max_records*params.record_size; i++, new_record_pos++) {
                new_dir.records[new_record_pos] = records[i];
            }
            for (int i = 0; i < params.record_size; i++, new_record_pos++) {
                new_dir.records[new_record_pos] = last_key[i];
            }
            // write dirs from middle_index + 1 to *count plus the last dir saved before
            int new_dir_pos = 0;
            for (int i = middle_index + 1; i <= *count; i++, new_dir_pos++) {
                new_dir.dirs[new_dir_pos] = dirs[i];
            }
            new_dir.dirs[new_dir_pos] = last_dir;
            // update counts
            *count = middle_index;
            *new_dir.count = params.dir_max_records - middle_index;

            // record at middle_index is returned
            std::unique_ptr<uint64_t[]> split_key = std::make_unique<uint64_t[]>(params.record_size);
            for (int i = 0; i < params.record_size; i++) {
                split_key[i] = records[middle_index*params.record_size + i];
            }
            return std::make_unique<std::pair<Record, int>>(Record(split_key.get(), params.record_size), new_page.page_number*-1);
        }
    }
    return nullptr;
}

void BPlusTreeDir::update_record(int index, Record& record)
{
    for (int i = 0; i < params.record_size; i++) {
        records[index*params.record_size + i] = record.ids[i];
    }
}

void BPlusTreeDir::update_dir(int index, int dir)
{
    dirs[index] = dir;
}

void BPlusTreeDir::rotate_records(int from, int to)
{
    for (int i = to; i >= from; i--) {
        for (int j = 0; j < params.record_size; j++) {
            records[(i+1)*params.record_size + j] = records[i*params.record_size + j];
        }
    }
}

void BPlusTreeDir::rotate_dirs(int from, int to)
{
    for (int i = to; i >= from; i--) {
        dirs[i+1] = dirs[i];
    }
}

std::pair<int, int> BPlusTreeDir::search_leaf(const Record& min)
{
    int dir_index = search_dir_index(0, *count, min);
    int page_pointer = dirs[dir_index];

    /*std::cout << "BPlusTreeDir::search_leaf\n";
    for (int i = 0; i < (*count)*2; i++) {
        std::cout << "   " << records[i++] << ", " << records[i] << "\n";
    }
    std::cout << "found " << dir_index << "\n";*/


    if (page_pointer < 0) { // negative number: pointer to dir
        Page& child_page = params.buffer_manager.get_page(page_pointer*-1, params.dir_path);
        BPlusTreeDir child =  BPlusTreeDir(params, child_page);
        return child.search_leaf(min);
    }
    else { // positive number: pointer to leaf
        Page& child_page = params.buffer_manager.get_page(page_pointer, params.leaf_path);
        BPlusTreeLeaf child =  BPlusTreeLeaf(params, child_page);
        return child.search_leaf(min);
    }
}

int BPlusTreeDir::search_dir_index(int dir_from, int dir_to, const Record& record)
{
    //std::cout << "BPlusTreeDir::search_dir_index from " << dir_from << " to " << dir_to << "\n";
    if (dir_from == dir_to) {
        return dir_from;
    }
    int middle_dir = (dir_from + dir_to + 1) / 2;
    int middle_record = middle_dir-1;

    for (int i = 0; i < params.record_size; i++) {
        auto id = records[middle_record*params.record_size + i];
        if (record.ids[i] < id) {
            return search_dir_index(dir_from, middle_record, record);
        }
        else if (record.ids[i] > id) {
            return search_dir_index(middle_record+1, dir_to, record);
        }
        // continue if they were equal
    }
    return search_dir_index(middle_record+1, dir_to, record);
}