#ifndef FILE__INDEX__B_PLUS_TREE_PARAMS_H_
#define FILE__INDEX__B_PLUS_TREE_PARAMS_H_

#include <string>
#include "file/page.h"

class BufferManager;

class BPlusTreeParams {
    public:
        BPlusTreeParams(BufferManager& buffer_manager, std::string path, int key_size);
        BPlusTreeParams(BufferManager& buffer_manager, std::string path, int key_size, int value_size);
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
