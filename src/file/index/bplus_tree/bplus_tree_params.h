#ifndef FILE__INDEX__B_PLUS_TREE__B_PLUS_TREE_PARAMS_
#define FILE__INDEX__B_PLUS_TREE__B_PLUS_TREE_PARAMS_

#include <string>
#include <boost/filesystem.hpp>
#include "file/page.h"

class BufferManager;

class BPlusTreeParams {
    public:
        BPlusTreeParams(BufferManager& buffer_manager, std::string path, int record_size)
            : buffer_manager(buffer_manager), dir_path(path + ".dir"), leaf_path(path + ".leaf"), record_size(record_size)
        {
            dir_max_records = 5;
            leaf_max_records = 5;
            is_empty = true; //TODO: detectar al crear
        }
        ~BPlusTreeParams() = default;

        BufferManager& buffer_manager;
        std::string const dir_path;
        std::string const leaf_path;
        int const record_size;

        int dir_max_records;
        int leaf_max_records;
        bool is_empty;
};

#endif //FILE__INDEX__B_PLUS_TREE__B_PLUS_TREE_PARAMS_
