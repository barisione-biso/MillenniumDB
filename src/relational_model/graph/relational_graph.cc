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

#define NODE_MASK  0x0100000000000000UL
#define EDGE_MASK  0x0200000000000000UL
#define LABEL_MASK 0x0300000000000000UL
#define KEY_MASK   0x0400000000000000UL
#define VALUE_MASK 0x0500000000000000UL
#define UNMASK     0x00FFFFFFFFFFFFFFUL
// TODO: ver posibles submasks de VALUE (int, char, float, string, date, bool)

using namespace std;

RelationalGraph::RelationalGraph(int graph_id, Config& config)
    : graph_id(graph_id), config(config)
{
    bpt_params_label2element = make_unique<BPlusTreeParams>(config.label2element_name, 2);
    bpt_params_element2label = make_unique<BPlusTreeParams>(config.element2label_name, 2);

    bpt_params_element2prop = make_unique<BPlusTreeParams>(config.element2prop_name, 3);
    bpt_params_prop2element = make_unique<BPlusTreeParams>(config.prop2element_name, 3);
    bpt_params_from_to_edge = make_unique<BPlusTreeParams>(config.from_to_edge_name, 3);
    bpt_params_to_from_edge = make_unique<BPlusTreeParams>(config.to_from_edge_name, 3);

    label2element = make_unique<BPlusTree>(*bpt_params_label2element);
    element2label = make_unique<BPlusTree>(*bpt_params_element2label);

    element2prop = make_unique<BPlusTree>(*bpt_params_element2prop);
    prop2element = make_unique<BPlusTree>(*bpt_params_prop2element);
    from_to_edge = make_unique<BPlusTree>(*bpt_params_from_to_edge);
    to_from_edge = make_unique<BPlusTree>(*bpt_params_to_from_edge);
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
Record RelationalGraph::get_record_for_edge(uint64_t node_from_id, uint64_t node_to_id, uint64_t edge_id) {
    Record record = Record(node_from_id|NODE_MASK, node_to_id|NODE_MASK, edge_id|EDGE_MASK);
    // config.get_catalog().add_node_label(record.ids[1] & UNMASK); // TODO: add to catalog?
    return record;
}

Record RelationalGraph::get_record_for_node_label(uint64_t node_id, const string& label) {
    Record record = get_record_for_element_label(node_id|NODE_MASK, label);
    config.get_catalog().add_node_label(record.ids[1] & UNMASK);
    return record;
}

Record RelationalGraph::get_record_for_edge_label(uint64_t edge_id, const string& label) {
    Record record = get_record_for_element_label(edge_id|EDGE_MASK, label);
    config.get_catalog().add_edge_label(record.ids[1] & UNMASK);
    return record;
}

Record RelationalGraph::get_record_for_node_property(uint64_t node_id, const string& key, const Value& value)
{
    Record record = get_record_for_element_property(node_id|NODE_MASK, key, value);
    config.get_catalog().add_node_key(record.ids[1] & UNMASK);
    return record;
}

Record RelationalGraph::get_record_for_edge_property(uint64_t edge_id, const string& key, const Value& value)
{
    Record record = get_record_for_element_property(edge_id|EDGE_MASK, key, value);
    config.get_catalog().add_edge_key(record.ids[1] & UNMASK);
    return record;
}

Record RelationalGraph::get_record_for_element_label(uint64_t element_id, const string& label) {
    int string_len = label.length();
    std::unique_ptr<std::vector<char>> label_bytes = std::make_unique<std::vector<char>>(string_len);
    std::copy(label.begin(), label.end(), (*label_bytes).begin());

    uint64_t label_id = get_or_create_id(std::move(label_bytes)) | LABEL_MASK;
    return Record(element_id, label_id);
}

Record RelationalGraph::get_record_for_element_property(uint64_t element_id, const string& key, const Value& value) {

    int string_len = key.length();
    std::unique_ptr<std::vector<char>> key_bytes = std::make_unique<std::vector<char>>(string_len);
    std::copy(key.begin(), key.end(), (*key_bytes).begin());

    uint64_t key_id = get_or_create_id(std::move(key_bytes)) | KEY_MASK;
    uint64_t value_id = get_or_create_id(value.get_bytes()) | VALUE_MASK;
    return Record(element_id, key_id, value_id);
}
/***************************************** End methods for bulk import *************************************/

/************************************* Methods to add elements one by one **********************************/
void RelationalGraph::connect_nodes(uint64_t id_from, uint64_t id_to, uint64_t id_edge) {
    from_to_edge->insert(
        Record(id_from|NODE_MASK,
               id_to|NODE_MASK,
               id_edge|EDGE_MASK)
    );
    to_from_edge->insert(
        Record(id_to|NODE_MASK,
               id_from|NODE_MASK,
               id_edge|EDGE_MASK)
    );
}

void RelationalGraph::add_label_to_node(uint64_t node_id, const string& label) {
    uint64_t label_id_masked = add_label(node_id|NODE_MASK, label);
    config.get_catalog().add_node_label(label_id_masked & UNMASK);
}

void RelationalGraph::add_label_to_edge(uint64_t edge_id, const string& label)
{
     uint64_t label_id_masked = add_label(edge_id|EDGE_MASK, label);
     config.get_catalog().add_edge_label(label_id_masked & UNMASK);
}

void RelationalGraph::add_property_to_node(uint64_t node_id, const string& key, const Value& value)
{
    uint64_t key_id_masked = add_property(node_id|NODE_MASK, key, value);
    config.get_catalog().add_node_key(key_id_masked & UNMASK);
}

void RelationalGraph::add_property_to_edge(uint64_t edge_id, const string& key, const Value& value)
{
    uint64_t key_id_masked = add_property(edge_id|EDGE_MASK, key, value);
    config.get_catalog().add_edge_key(key_id_masked & UNMASK);
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

uint64_t RelationalGraph::add_label(uint64_t id, const string& label) {
    int string_len = label.length();
    std::unique_ptr<std::vector<char>> label_bytes = std::make_unique<std::vector<char>>(string_len);
    std::copy(label.begin(), label.end(), (*label_bytes).begin());

    uint64_t label_id = get_or_create_id(std::move(label_bytes)) | LABEL_MASK;
    label2element->insert( Record(label_id, id) );
    element2label->insert( Record(id, label_id) );
    return label_id;
}

uint64_t RelationalGraph::add_property(uint64_t id, const string& key, const Value& value) {
    int string_len = key.length();
    std::unique_ptr<std::vector<char>> key_bytes = std::make_unique<std::vector<char>>(string_len);
    std::copy(key.begin(), key.end(), (*key_bytes).begin());

    uint64_t key_id = get_or_create_id(std::move(key_bytes)) | KEY_MASK;
    uint64_t value_id = get_or_create_id(value.get_bytes()) | VALUE_MASK;
    element2prop->insert( Record(id, key_id, value_id) );
    prop2element->insert( Record(key_id, value_id, id) );
    return key_id;
}
/********************************** End methods to add elements one by one *********************************/


Node RelationalGraph::get_node(uint64_t id) {
    return Node(id & UNMASK);
}

Label RelationalGraph::get_label(uint64_t id) {
    auto bytes = config.get_object_file().read(id&UNMASK);
    string label_name(bytes->begin(), bytes->end());
    return Label(label_name);
}

Key RelationalGraph::get_key(uint64_t id) {
    auto bytes = config.get_object_file().read(id&UNMASK);
    string key_name(bytes->begin(), bytes->end());
    return Key(key_name);
}

unique_ptr<Value> RelationalGraph::get_value(uint64_t id) {
    auto bytes = config.get_object_file().read(id&UNMASK);
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
        cout << "ERROR: Label doesn't exist\n";
        exit(1);
    }
    else { // label_name already exists
        label_id = next->ids[2];
    }
    return ObjectId(graph_id, label_id|LABEL_MASK);
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
        cout << "ERROR: Key doesn't exist";
        exit(1);
    }
    else { // label_name already exists
        key_id = next->ids[2];
    }
    return ObjectId(graph_id, key_id|KEY_MASK);
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
        cout << "ERROR: Value doesn't exist";
        exit(1);
    }
    else { // label_name already exists
        value_id = next->ids[2];
    }
    return ObjectId(graph_id, value_id|VALUE_MASK);
}
