#ifndef RELATIONAL_MODEL__CONFIG_H_
#define RELATIONAL_MODEL__CONFIG_H_

#include "file/buffer_manager.h"
#include "file/catalog/catalog.h"
#include "file/index/bplus_tree/bplus_tree.h"
#include "file/index/bplus_tree/bplus_tree_dir.h"
#include "file/index/bplus_tree/bplus_tree_params.h"
#include "file/index/object_file/object_file.h"

#include <memory>


class Config {
public:
    Config() {
        cout << "Buffer Manager ready\n";
        object_file = std::make_unique<ObjectFile>(object_file_name);
        cout << "Object file ready\n";
        catalog = std::make_unique<Catalog>(catalog_file_name);
        cout << "Catalog ready\n";
        bpt_params_hash2id = std::make_unique<BPlusTreeParams>(hash2id_name, 3); // Hash:128 + Key:64
        hash2id = std::make_unique<BPlusTree>(*bpt_params_hash2id);
        cout << "hash2id ready\n";
    }
    ~Config() {
        std::cout << "~Config\n";
    }

    ObjectFile&     get_object_file()    { return *object_file; }
    Catalog&        get_catalog()        { return *catalog; }
    BPlusTree&      get_hash2id_bpt()    { return *hash2id; }

    const std::string object_file_name    = "objects.dat";
    const std::string catalog_file_name   = "catalog.dat";
    const std::string hash2id_name        = "hash2id";
    const std::string label2element_name  = "label2element";
    const std::string element2label_name  = "element2label";
    const std::string element2prop_name   = "element2prop";
    const std::string prop2element_name   = "prop2element";
    const std::string from_to_edge_name   = "from_to_edge";
    const std::string to_from_edge_name   = "to_from_edge";

private:
    unique_ptr<ObjectFile>       object_file;
    unique_ptr<Catalog>          catalog;
    unique_ptr<BPlusTree>        hash2id; // ObjectHash|ObjectId.
    unique_ptr<BPlusTreeParams>  bpt_params_hash2id;
};

#endif //RELATIONAL_MODEL__CONFIG_H_
