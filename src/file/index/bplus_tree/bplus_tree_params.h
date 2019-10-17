#ifndef FILE__INDEX__B_PLUS_TREE_PARAMS_H_
#define FILE__INDEX__B_PLUS_TREE_PARAMS_H_

#include <string>
#include "file/page.h"

class BufferManager;

class BPlusTreeParams {
    public:
        BPlusTreeParams(BufferManager& buffer_manager, std::string path, int key_size)
            : buffer_manager(buffer_manager), dir_path(path + ".dir"), leaf_path(path + ".leaf"),
            key_size(key_size), value_size(0), total_size(key_size)
        {
            dir_max_records = 5;
            leaf_max_records = 5;
        }

        BPlusTreeParams(BufferManager& buffer_manager, std::string path, int key_size, int value_size)
            : buffer_manager(buffer_manager), dir_path(path + ".dir"), leaf_path(path + ".leaf"),
            key_size(key_size), value_size(value_size), total_size(key_size+value_size)
        {
            dir_max_records = 5;
            leaf_max_records = 5;
        }

        ~BPlusTreeParams() = default;

        BufferManager& buffer_manager;
        std::string const dir_path;
        std::string const leaf_path;

        int const key_size;
        int const value_size;
        int const total_size;

        int dir_max_records;
        int leaf_max_records;
};

#endif //FILE__INDEX__B_PLUS_TREE_PARAMS_H_
