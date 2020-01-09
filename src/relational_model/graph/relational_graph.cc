#include "relational_model/graph/relational_graph.h"

#include <openssl/md5.h>

#include "base/graph/edge.h"
#include "base/graph/node.h"
#include "base/graph/label.h"
#include "base/graph/property.h"
#include "base/graph/value/value_string.h"
#include "file/buffer_manager.h"
#include "file/index/record.h"
#include "file/index/bplus_tree/bplus_tree.h"
#include "relational_model/config.h"

// #define NODE_MASK  0x0100000000000000UL
// #define EDGE_MASK  0x0200000000000000UL
// #define LABEL_MASK 0x0300000000000000UL
// #define KEY_MASK   0x0400000000000000UL
// #define VALUE_MASK 0x0500000000000000UL
// #define UNMASK     0x00FFFFFFFFFFFFFFUL
// TODO: ver posibles submasks de VALUE (int, char, float, string, date, bool)

using namespace std;

RelationalGraph::RelationalGraph(int graph_id, Config& config)
    : graph_id(graph_id), config(config)
{
    bpt_params_label2node = make_unique<BPlusTreeParams>(config.label2node_name, 2);
    bpt_params_label2edge = make_unique<BPlusTreeParams>(config.label2edge_name, 2);
    bpt_params_node2label = make_unique<BPlusTreeParams>(config.node2label_name, 2);
    bpt_params_edge2label = make_unique<BPlusTreeParams>(config.edge2label_name, 2);

    bpt_params_prop2node = make_unique<BPlusTreeParams>(config.prop2node_name, 3);
    bpt_params_prop2edge = make_unique<BPlusTreeParams>(config.prop2edge_name, 3);
    bpt_params_node2prop = make_unique<BPlusTreeParams>(config.node2prop_name, 3);
    bpt_params_edge2prop = make_unique<BPlusTreeParams>(config.edge2prop_name, 3);

    bpt_params_from_to_edge = make_unique<BPlusTreeParams>(config.from_to_edge_name, 3);
    bpt_params_to_edge_from = make_unique<BPlusTreeParams>(config.to_edge_from_name, 3);
    bpt_params_edge_from_to = make_unique<BPlusTreeParams>(config.edge_from_to_name, 3);

    label2node = make_unique<BPlusTree>(*bpt_params_label2node);
    label2edge = make_unique<BPlusTree>(*bpt_params_label2edge);
    node2label = make_unique<BPlusTree>(*bpt_params_node2label);
    edge2label = make_unique<BPlusTree>(*bpt_params_edge2label);

    prop2node = make_unique<BPlusTree>(*bpt_params_prop2node);
    prop2edge = make_unique<BPlusTree>(*bpt_params_prop2edge);
    node2prop = make_unique<BPlusTree>(*bpt_params_node2prop);
    edge2prop = make_unique<BPlusTree>(*bpt_params_edge2prop);

    from_to_edge = make_unique<BPlusTree>(*bpt_params_from_to_edge);
    to_edge_from = make_unique<BPlusTree>(*bpt_params_to_edge_from);
    edge_from_to = make_unique<BPlusTree>(*bpt_params_edge_from_to);
}

RelationalGraph::~RelationalGraph() {
    std::cout << "~RelationalGraph\n";
}

uint64_t RelationalGraph::create_node() {
    return config.get_catalog().create_node();
}

uint64_t RelationalGraph::create_edge() {
    return config.get_catalog().create_edge();
}

/******************************************* Methods for bulk import ***************************************/
Record RelationalGraph::get_record_for_node_label(uint64_t node_id, const string& label) {
    uint64_t label_id = get_or_create_id(label);
    config.get_catalog().add_node_label(label_id);
    return Record(node_id, label_id);
}

Record RelationalGraph::get_record_for_edge_label(uint64_t edge_id, const string& label) {
    uint64_t label_id = get_or_create_id(label);
    config.get_catalog().add_edge_label(label_id);
    return Record(edge_id, label_id);
}

Record RelationalGraph::get_record_for_node_property(uint64_t node_id, const string& key, const Value& value)
{
    uint64_t key_id = get_or_create_id(key);
    uint64_t value_id = get_or_create_id(value.get_bytes());

    config.get_catalog().add_node_key(key_id);
    return Record(node_id, key_id, value_id);

}

Record RelationalGraph::get_record_for_edge_property(uint64_t edge_id, const string& key, const Value& value)
{
    uint64_t key_id = get_or_create_id(key);
    uint64_t value_id = get_or_create_id(value.get_bytes());

    config.get_catalog().add_edge_key(key_id);
    return Record(edge_id, key_id, value_id);
}

/***************************************** End methods for bulk import *************************************/

/************************************* Methods to add elements one by one **********************************/
void RelationalGraph::connect_nodes(uint64_t id_from, uint64_t id_to, uint64_t id_edge) {
    from_to_edge->insert( Record(id_from, id_to, id_edge) );
    to_edge_from->insert( Record(id_to, id_edge, id_from) );
    edge_from_to->insert( Record(id_edge, id_from, id_to) );
}


void RelationalGraph::add_label_to_node(uint64_t node_id, const string& label) {
    uint64_t label_id = get_or_create_id(label);

    label2node->insert( Record(label_id, node_id) );
    node2label->insert( Record(node_id, label_id) );

    config.get_catalog().add_node_label(label_id);
}


void RelationalGraph::add_label_to_edge(uint64_t edge_id, const string& label) {
    // int string_len = label.length();
    // std::unique_ptr<std::vector<char>> label_bytes = std::make_unique<std::vector<char>>(string_len);
    // std::copy(label.begin(), label.end(), (*label_bytes).begin());

    // uint64_t label_id = get_or_create_id(std::move(label_bytes));
    uint64_t label_id = get_or_create_id(label);

    label2edge->insert( Record(label_id, edge_id) );
    edge2label->insert( Record(edge_id, label_id) );

    config.get_catalog().add_edge_label(label_id);
}

void RelationalGraph::add_property_to_node(uint64_t node_id, const string& key, const Value& value) {
    uint64_t key_id   = get_or_create_id(key);
    uint64_t value_id = get_or_create_id(value.get_bytes());

    node2prop->insert( Record(node_id, key_id, value_id) );
    prop2node->insert( Record(key_id, value_id, node_id) );

    config.get_catalog().add_node_key(key_id);
}

void RelationalGraph::add_property_to_edge(uint64_t edge_id, const string& key, const Value& value) {
    uint64_t key_id   = get_or_create_id(key);
    uint64_t value_id = get_or_create_id(value.get_bytes());

    edge2prop->insert( Record(edge_id, key_id, value_id) );
    prop2edge->insert( Record(key_id, value_id, edge_id) );

    config.get_catalog().add_edge_key(key_id);
}

uint64_t RelationalGraph::get_or_create_id(unique_ptr< vector<char> > obj_bytes) {
    uint64_t hash[2];
    MD5((const unsigned char*)obj_bytes->data(), obj_bytes->size(), (unsigned char *)hash);

    // check if bpt contains object
    BPlusTree& hash2id = config.get_hash2id_bpt();
    auto iter = hash2id.get_range(
        Record(hash[0], hash[1], 0),
        Record(hash[0], hash[1], UINT64_MAX)
    );
    auto next = iter->next();
    if (next == nullptr) { // obj doesn't exist
        // Insert in object file
        uint64_t obj_id = config.get_object_file().write(*obj_bytes);
        // Insert in bpt
        hash2id.insert( Record(hash[0], hash[1], obj_id) );
        return obj_id;
    }
    else { // obj already exists
        return next->ids[2];
    }
}

uint64_t RelationalGraph::get_or_create_id(const string& str) {
    int string_len = str.length();
    std::unique_ptr<std::vector<char>> bytes = std::make_unique<std::vector<char>>(string_len);
    std::copy(str.begin(), str.end(), (*bytes).begin());

    return get_or_create_id(std::move(bytes));
}

/********************************** End methods to add elements one by one *********************************/


Node RelationalGraph::get_node(uint64_t id) {
    return Node(id);
}

Label RelationalGraph::get_label(uint64_t id) {
    auto bytes = config.get_object_file().read(id);
    string label_name(bytes->begin(), bytes->end());
    return Label(label_name);
}

Key RelationalGraph::get_key(uint64_t id) {
    auto bytes = config.get_object_file().read(id);
    string key_name(bytes->begin(), bytes->end());
    return Key(key_name);
}

unique_ptr<Value> RelationalGraph::get_value(uint64_t id) {
    auto bytes = config.get_object_file().read(id);
    string str(bytes->begin(), bytes->end());
    return make_unique<ValueString>(str);
}

ObjectId RelationalGraph::get_label_id(Label const& label) {
    uint64_t hash[2]; // check MD5_DIGEST_LENGTH == 16?
    string label_name = label.get_label_name();
    MD5((const unsigned char*)label_name.c_str(), label_name.size(), (unsigned char *)hash);

    uint64_t label_id;

    // check if bpt contains object
    BPlusTree& bpt = config.get_hash2id_bpt();
    auto iter = bpt.get_range(
        Record(hash[0], hash[1], 0),
        Record(hash[0], hash[1], UINT64_MAX)
    );
    auto next = iter->next();
    if (next == nullptr) { // label_name doesn't exist
        return ObjectId::get_not_found();
    }
    else { // label_name already exists
        label_id = next->ids[2];
    }
    return ObjectId(label_id);
}

ObjectId RelationalGraph::get_key_id(Key const& key)
{
    uint64_t hash[2]; // check MD5_DIGEST_LENGTH == 16?
    string key_name = key.get_key_name();
    MD5((const unsigned char*)key_name.c_str(), key_name.size(), (unsigned char *)hash);

    uint64_t key_id;

    // check if bpt contains object
    BPlusTree& bpt = config.get_hash2id_bpt();
    auto iter = bpt.get_range(
        Record(hash[0], hash[1], 0),
        Record(hash[0], hash[1], UINT64_MAX)
    );
    auto next = iter->next();
    if (next == nullptr) { // key_name doesn't exist
        return ObjectId::get_not_found();
    }
    else { // label_name already exists
        key_id = next->ids[2];
    }
    return ObjectId(key_id);
}

ObjectId RelationalGraph::get_value_id(Value const& value)
{
    uint64_t hash[2]; // check MD5_DIGEST_LENGTH == 16?
    auto bytes = value.get_bytes();
    MD5((const unsigned char*)bytes->data(), bytes->size(), (unsigned char *)hash);

    uint64_t value_id;

    // check if bpt contains object
    BPlusTree& bpt = config.get_hash2id_bpt();
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
