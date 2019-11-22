#include "file/index/bplus_tree/bplus_tree_params.h"

BPlusTreeParams::BPlusTreeParams(std::string path, int key_size)
    : dir_path(path + ".dir"), leaf_path(path + ".leaf"),
    key_size(key_size), value_size(0), total_size(key_size)
{
    // PAGE_SIZE >= int_size(4) + ulong_size(8)*dir_max_records*key_size + int_size(4)*(dir_max_records+1)
    // PAGE_SIZE >= 4 + 8*dir_max_records*key_size + 4*dir_max_records + 4
    // PAGE_SIZE - 8 >= dir_max_records * (8*key_size + 4)
    // (PAGE_SIZE - 8)/(8*key_size + 4) >= dir_max_records
    dir_max_records = (PAGE_SIZE - 8)/(8*key_size + 4);

    // PAGE_SIZE >= 2*int_size(4) + ulong_size(8)*leaf_max_records*total_size
    // PAGE_SIZE >= 8 + 8*leaf_max_records*total_size
    // (PAGE_SIZE - 8)/(8*total_size) >= leaf_max_records
    leaf_max_records = (PAGE_SIZE - 8)/(8*total_size);
}

BPlusTreeParams::BPlusTreeParams(std::string path, int key_size, int value_size)
    : dir_path(path + ".dir"), leaf_path(path + ".leaf"),
    key_size(key_size), value_size(value_size), total_size(key_size+value_size)
{
    dir_max_records = (PAGE_SIZE - 8)/(8*key_size + 4);
    leaf_max_records = (PAGE_SIZE - 8)/(8*total_size);
}

