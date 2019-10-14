#ifndef RELATIONAL_MODEL__CONFIG_H_
#define RELATIONAL_MODEL__CONFIG_H_

#include "file/buffer_manager.h"
#include "file/index/bplus_tree/bplus_tree.h"
#include "file/index/bplus_tree/bplus_tree_dir.h"
#include "file/index/bplus_tree/bplus_tree_params.h"
#include "file/index/object_file/object_file.h"

#include <memory>

class Config {
public:
    Config() {
        buffer_manager = std::make_unique<BufferManager>();
        object_file = std::make_unique<ObjectFile>(get_path(object_file_name));
        bpt_params_hash2id = std::make_unique<BPlusTreeParams>(*buffer_manager, get_path(hash2id_name), 3); // Hash:128 + Key:64
        hash2id = std::make_unique<BPlusTree>(*bpt_params_hash2id);
    }
    ~Config() = default;

    BufferManager& get_buffer_manager() { return *buffer_manager; }

    string get_path(string filename) {
        return "test_files/" + filename;
    }

    const std::string object_file_name    = "objects.dat";
    const std::string hash2id_name        = "hash2id";
    const std::string label2element_name  = "label2element";
    const std::string element2label_name  = "element2label";
    const std::string element2prop_name   = "element2prop";
    const std::string prop2element_name   = "prop2element";
    const std::string from_to_edge_name   = "from_to_edge";
    const std::string to_from_edge_name   = "to_from_edge";

    ObjectFile& get_object_file() { return *object_file; }
    BPlusTree& get_hash2id_bpt() { return *hash2id; }

private:
    unique_ptr<BufferManager> buffer_manager;
    unique_ptr<ObjectFile> object_file;  // No es del grafo sino que del sistema
    unique_ptr<BPlusTree> hash2id;       // ObjectHash|ObjectId. No es del grafo sino que del sistema
    unique_ptr<BPlusTreeParams> bpt_params_hash2id;
};

#endif //RELATIONAL_MODEL__CONFIG_H_
