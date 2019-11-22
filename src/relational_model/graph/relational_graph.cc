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

using namespace std;

RelationalGraph::RelationalGraph(int graph_id, Config& config)
    : graph_id(graph_id), config(config)
{
    bpt_params_label2element = make_unique<BPlusTreeParams>(config.get_path(config.label2element_name), 2);
    bpt_params_element2label = make_unique<BPlusTreeParams>(config.get_path(config.element2label_name), 2);

    bpt_params_element2prop = make_unique<BPlusTreeParams>(config.get_path(config.element2prop_name), 3);
    bpt_params_prop2element = make_unique<BPlusTreeParams>(config.get_path(config.prop2element_name), 3);
    bpt_params_from_to_edge = make_unique<BPlusTreeParams>(config.get_path(config.from_to_edge_name), 3);
    bpt_params_to_from_edge = make_unique<BPlusTreeParams>(config.get_path(config.to_from_edge_name), 3);

    label2element = make_unique<BPlusTree>(*bpt_params_label2element);
    element2label = make_unique<BPlusTree>(*bpt_params_element2label);

    element2prop = make_unique<BPlusTree>(*bpt_params_element2prop);
    prop2element = make_unique<BPlusTree>(*bpt_params_prop2element);
    from_to_edge = make_unique<BPlusTree>(*bpt_params_from_to_edge);
    to_from_edge = make_unique<BPlusTree>(*bpt_params_to_from_edge);
}

RelationalGraph::~RelationalGraph() {
    std::cout << "~RelationalGraph\n";
    BufferManager::flush();
}


uint64_t RelationalGraph::create_node()
{
    return config.get_catalog().create_node();
}

uint64_t RelationalGraph::create_edge()
{
    return config.get_catalog().create_edge();
}

void RelationalGraph::connect_nodes(uint64_t id_from, uint64_t id_to, uint64_t id_edge)
{
    from_to_edge->insert(
        Record(
            id_from|NODE_MASK,
            id_to|NODE_MASK,
            id_edge|EDGE_MASK
        )
    );
    to_from_edge->insert(
        Record(
            id_to|NODE_MASK,
            id_from|NODE_MASK,
            id_edge|EDGE_MASK
        )
    );
}

void RelationalGraph::add_label_to_node(uint64_t node_id, const string& label)
{
    uint64_t label_id_unmasked = add_label(node_id|NODE_MASK, label);
    config.get_catalog().add_node_label(label_id_unmasked);
}

void RelationalGraph::add_label_to_edge(uint64_t edge_id, const string& label)
{
     uint64_t label_id_unmasked = add_label(edge_id|EDGE_MASK, label);
     config.get_catalog().add_edge_label(label_id_unmasked);
}

void RelationalGraph::add_property_to_node(uint64_t node_id, const string& key, const Value& value)
{
    uint64_t key_id_unmasked = add_property(node_id|NODE_MASK, key, value);
    config.get_catalog().add_node_key(key_id_unmasked);
}

void RelationalGraph::add_property_to_edge(uint64_t edge_id, const string& key, const Value& value)
{
    uint64_t key_id_unmasked = add_property(edge_id|EDGE_MASK, key, value);
    config.get_catalog().add_edge_key(key_id_unmasked);
}

uint64_t RelationalGraph::add_label(uint64_t id, const string& label)
{
    uint64_t hash[2]; // check MD5_DIGEST_LENGTH == 16?
    MD5((const unsigned char*)label.c_str(), label.size(), (unsigned char *)hash);

    uint64_t label_id;

    // check if bpt contains object
    BPlusTree& bpt = config.get_hash2id_bpt();
    auto iter = bpt.get_range(
        Record(hash[0], hash[1], 0),
        Record(hash[0], hash[1], UINT64_MAX)
    );
    auto next = iter->next();
    if (next == nullptr) { // label_name doesn't exist
        // insert in object file
        vector<char> vect(label.length());
	    copy(label.begin(), label.end(), vect.begin());
        label_id = config.get_object_file().write(vect);

        // insert in bpt
        bpt.insert(
            Record(
                hash[0], hash[1], label_id // NO MASK
            )
        );
    }
    else { // label_name already exists
        label_id = next->ids[2];
    }

    label2element->insert(
        Record(
            label_id|LABEL_MASK,
            id
        )
    );
    element2label->insert(
        Record(
            id,
            label_id|LABEL_MASK
        )
    );

    return label_id;
}


uint64_t RelationalGraph::add_property(uint64_t id, const string& key, const Value& value)
{
    uint64_t hash_key[2];
    MD5((const unsigned char*)key.c_str(), key.size(), (unsigned char *)hash_key);

    uint64_t hash_value[2];
    auto value_bytes = value.get_bytes();
    MD5((const unsigned char*)value_bytes->data(), value_bytes->size(), (unsigned char *)hash_value);

    uint64_t key_id;
    uint64_t value_id;

    // check if bpt contains key
    BPlusTree& bpt = config.get_hash2id_bpt();
    auto iter = bpt.get_range(
        Record(hash_key[0], hash_key[1], 0),
        Record(hash_key[0], hash_key[1], UINT64_MAX)
    );
    auto next = iter->next();
    if (next == nullptr) { // key doesn't exist
        // insert in object file
        vector<char> vect(key.length());
	    copy(key.begin(), key.end(), vect.begin());
        key_id = config.get_object_file().write(vect);

        // insert in bpt
        bpt.insert(
            Record(hash_key[0], hash_key[1], key_id)
        );
    }
    else { // label_name already exists
        key_id = next->ids[2];
    }

    // check if bpt contains value
    iter = bpt.get_range(
        Record(hash_value[0], hash_value[1], 0),
        Record(hash_value[0], hash_value[1], UINT64_MAX)
    );
    next = iter->next();
    if (next == nullptr) { // value doesn't exist
        // insert in object file
        value_id = config.get_object_file().write(*value_bytes);

        // insert in bpt
        bpt.insert(
            Record(hash_value[0], hash_value[1], value_id)
        );
    }
    else { // value already exists
        value_id = next->ids[2];
    }

    element2prop->insert(
        Record(
            id,
            key_id|KEY_MASK,
            value_id|VALUE_MASK
        )
    );
    prop2element->insert(
        Record(
            key_id|KEY_MASK,
            value_id|VALUE_MASK,
            id
        )
    );

    return key_id;
}

Node RelationalGraph::get_node(uint64_t id)
{
    return Node(id & UNMASK);
}

Label RelationalGraph::get_label(uint64_t id)
{
    auto bytes = config.get_object_file().read(id&UNMASK);
    string label_name(bytes->begin(), bytes->end());
    return Label(label_name);
}

Key RelationalGraph::get_key(uint64_t id)
{
    auto bytes = config.get_object_file().read(id&UNMASK);
    string key_name(bytes->begin(), bytes->end());
    return Key(key_name);
}

unique_ptr<Value> RelationalGraph::get_value(uint64_t id)
{
    auto bytes = config.get_object_file().read(id&UNMASK);
    string str(bytes->begin(), bytes->end());
    return make_unique<ValueString>(str);
}

ObjectId RelationalGraph::get_label_id(Label const& label)
{
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
