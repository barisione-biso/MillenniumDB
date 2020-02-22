#include "relational_model.h"

#include "base/graph/value/value_int.h"
#include "base/graph/value/value_string.h"
#include "relational_model/graph/relational_graph.h"

#include <openssl/md5.h>

using namespace std;

unique_ptr<RelationalModel> RelationalModel::instance = nullptr; // can't use static object because dependency with BufferManager
// TODO: need to destruct catalog to write to disk

RelationalModel::RelationalModel() {
    object_file = make_unique<ObjectFile>(object_file_name);
    catalog = make_unique<Catalog>(catalog_file_name);
    auto bpt_params_hash2id = make_unique<BPlusTreeParams>(hash2id_name, 3); // Hash:2*64 + Key:64
    hash2id = make_unique<BPlusTree>(move(bpt_params_hash2id));
}


void RelationalModel::init() {
    instance = std::make_unique<RelationalModel>();
}


ObjectId RelationalModel::get_id(const string& str) {
    uint64_t hash[2]; // check MD5_DIGEST_LENGTH == 16?
    MD5((const unsigned char*)str.c_str(), str.size(), (unsigned char *)hash);

    uint64_t id;

    // check if bpt contains object
    BPlusTree& bpt = get_hash2id_bpt();
    auto iter = bpt.get_range(
        Record(hash[0], hash[1], 0),
        Record(hash[0], hash[1], UINT64_MAX)
    );
    auto next = iter->next();
    if (next == nullptr) { // label_name doesn't exist
        return ObjectId::get_not_found();
    }
    else { // label_name already exists
        id = next->ids[2];
    }
    return ObjectId(id);
}


ObjectId RelationalModel::get_id(Value const& value) {
    uint64_t hash[2]; // check MD5_DIGEST_LENGTH == 16?
    auto bytes = value.get_bytes();
    MD5((const unsigned char*)bytes->data(), bytes->size(), (unsigned char *)hash);

    uint64_t value_id;

    // check if bpt contains object
    BPlusTree& bpt = get_hash2id_bpt();
    auto iter = bpt.get_range(
        Record(hash[0], hash[1], 0),
        Record(hash[0], hash[1], UINT64_MAX)
    );
    auto next = iter->next();
    if (next == nullptr) {
        return ObjectId::get_not_found();
    }
    else { // label_name already exists
        value_id = next->ids[2];
    }
    return ObjectId(value_id);
}


uint64_t RelationalModel::get_or_create_id(unique_ptr< vector<char> > obj_bytes) {
    uint64_t hash[2];
    MD5((const unsigned char*)obj_bytes->data(), obj_bytes->size(), (unsigned char *)hash);

    // check if bpt contains object
    BPlusTree& hash2id = RelationalModel::get_hash2id_bpt();
    auto iter = hash2id.get_range(
        Record(hash[0], hash[1], 0),
        Record(hash[0], hash[1], UINT64_MAX)
    );
    auto next = iter->next();
    if (next == nullptr) { // obj doesn't exist
        // Insert in object file
        uint64_t obj_id = RelationalModel::get_object_file().write(*obj_bytes);
        // Insert in bpt
        hash2id.insert( Record(hash[0], hash[1], obj_id) );
        return obj_id;
    }
    else { // obj already exists
        return next->ids[2];
    }
}


uint64_t RelationalModel::get_or_create_id(const string& str) {
    int string_len = str.length();
    std::unique_ptr<std::vector<char>> bytes = std::make_unique<std::vector<char>>(string_len);
    std::copy(str.begin(), str.end(), (*bytes).begin());

    return get_or_create_id(std::move(bytes));
}


shared_ptr<Value> RelationalModel::get_value(ObjectId object_id) {
    auto bytes = instance->object_file->read(object_id);
    auto prefix = object_id >> 56;
    if (prefix == VALUE_STR_MASK >> 56) {
        string value_string(bytes->begin(), bytes->end());
        return make_shared<ValueString>(move(value_string));
    }
    else if (prefix == VALUE_INT_MASK >> 56) {
        int i = 0;
        return make_shared<ValueInt>(i);
    }
    else {
        throw logic_error("wrong value prefix.");
    }
}


RelationalGraph& RelationalModel::get_graph(GraphId graph_id) {
    auto search = instance->graphs.find(graph_id);
    if (search != instance->graphs.end()) {
        return *search->second.get();
    }
    else {
        instance->graphs.insert({ graph_id, make_unique<RelationalGraph>(graph_id) });
        return *instance->graphs[graph_id].get();
    }
}

ObjectFile& RelationalModel::get_object_file() { return *instance->object_file; }
Catalog&    RelationalModel::get_catalog()     { return *instance->catalog; }
BPlusTree&  RelationalModel::get_hash2id_bpt() { return *instance->hash2id; }