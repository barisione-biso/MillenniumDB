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

private:
    static std::unique_ptr<Config> instance;

    unique_ptr<ObjectFile>       object_file;
    unique_ptr<Catalog>          catalog;
    unique_ptr<BPlusTree>        hash2id; // ObjectHash|ObjectId.
    unique_ptr<BPlusTreeParams>  bpt_params_hash2id;

    

public:

    // constructor should be private but needed for make_unique
    Config() {
        object_file = std::make_unique<ObjectFile>(object_file_name);
        catalog = std::make_unique<Catalog>(catalog_file_name);
        bpt_params_hash2id = std::make_unique<BPlusTreeParams>(hash2id_name, 3); // Hash:2*64 + Key:64
        hash2id = std::make_unique<BPlusTree>(*bpt_params_hash2id);
    }

    ~Config() { }

    static void init() {
        instance = std::make_unique<Config>();
    }

    static ObjectFile& get_object_file()    { return *instance->object_file; }
    static Catalog&    get_catalog()        { return *instance->catalog; }
    static BPlusTree&  get_hash2id_bpt()    { return *instance->hash2id; }

    const std::string object_file_name    = "objects.dat";
    const std::string catalog_file_name   = "catalog.dat";
    const std::string hash2id_name        = "hash_id";

    // For labels
    const std::string label2node_name  = "LN";
    const std::string node2label_name  = "NL";
    const std::string label2edge_name  = "LE";
    const std::string edge2label_name  = "NL";

    const std::string node2prop_name   = "NKV";
    const std::string prop2node_name   = "KVN";
    const std::string edge2prop_name   = "EKV";
    const std::string prop2edge_name   = "KVE";

    const std::string from_to_edge_name   = "FTE";
    const std::string to_edge_from_name   = "TEF";
    const std::string edge_from_to_name   = "EFT";
};

#endif //RELATIONAL_MODEL__CONFIG_H_
