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
    BufferManager& buffer_manager = config.get_buffer_manager();

    bpt_params_label2element = make_unique<BPlusTreeParams>(buffer_manager, config.get_path(config.label2element_name), 2);
    bpt_params_element2label = make_unique<BPlusTreeParams>(buffer_manager, config.get_path(config.element2label_name), 2);

    bpt_params_element2prop = make_unique<BPlusTreeParams>(buffer_manager, config.get_path(config.element2prop_name), 3);
    bpt_params_prop2element = make_unique<BPlusTreeParams>(buffer_manager, config.get_path(config.prop2element_name), 3);
    bpt_params_from_to_edge = make_unique<BPlusTreeParams>(buffer_manager, config.get_path(config.from_to_edge_name), 3);
    bpt_params_to_from_edge = make_unique<BPlusTreeParams>(buffer_manager, config.get_path(config.to_from_edge_name), 3);

    label2element = make_unique<BPlusTree>(*bpt_params_label2element);
    element2label = make_unique<BPlusTree>(*bpt_params_element2label);

    element2prop = make_unique<BPlusTree>(*bpt_params_element2prop);
    prop2element = make_unique<BPlusTree>(*bpt_params_prop2element);
    from_to_edge = make_unique<BPlusTree>(*bpt_params_from_to_edge);
    to_from_edge = make_unique<BPlusTree>(*bpt_params_to_from_edge);
}


Node RelationalGraph::create_node(string const& id)
{
    u_int64_t new_id = config.get_catalog().create_node(id);
    return Node(new_id & UNMASK);
}


Edge RelationalGraph::connect_nodes(Node& from, Node& to)
{
    u_int64_t edge_id = config.get_catalog().create_edge();
    from_to_edge->insert(
        Record(
            from.get_id()|NODE_MASK,
            to.get_id()|NODE_MASK,
            edge_id|EDGE_MASK
        )
    );

    to_from_edge->insert(
        Record(
            from.get_id()|NODE_MASK,
            to.get_id()|NODE_MASK,
            edge_id|EDGE_MASK
        )
    );

    return Edge(edge_id);
}


void RelationalGraph::add_label(GraphElement& element, Label const& label)
{
    u_int64_t hash[2]; // check MD5_DIGEST_LENGTH == 16?
    string label_name = label.get_label_name();
    MD5((const unsigned char*)label_name.c_str(), label_name.size(), (unsigned char *)hash);

    u_int64_t label_id;

    // check if bpt contains object
    BPlusTree& bpt = config.get_hash2id_bpt();
    auto iter = bpt.get_range(
        Record(hash[0], hash[1], 0),
        Record(hash[0], hash[1], UINT64_MAX)
    );
    auto next = iter->next();
    if (next == nullptr) { // label_name doesn't exist
        // insert in object file
        vector<char> vect(label_name.length());
	    copy(label_name.begin(), label_name.end(), vect.begin());
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

    u_int64_t mask = element.is_node() ? NODE_MASK : EDGE_MASK;
    label2element->insert(
        Record(
            label_id|LABEL_MASK,
            element.get_id()|mask
        )
    );
    element2label->insert(
        Record(
            element.get_id()|mask,
            label_id|LABEL_MASK
        )
    );
}


void RelationalGraph::add_property(GraphElement& element, Property const& property)
{
    u_int64_t hash_key[2];
    string key_name = property.get_key().get_key_name();
    MD5((const unsigned char*)key_name.c_str(), key_name.size(), (unsigned char *)hash_key);

    u_int64_t hash_value[2];
    auto value = property.get_value().get_bytes();
    MD5((const unsigned char*)value->data(), value->size(), (unsigned char *)hash_value);

    u_int64_t key_id;
    u_int64_t value_id;

    // check if bpt contains key
    BPlusTree& bpt = config.get_hash2id_bpt();
    auto iter = bpt.get_range(
        Record(hash_key[0], hash_key[1], 0),
        Record(hash_key[0], hash_key[1], UINT64_MAX)
    );
    auto next = iter->next();
    if (next == nullptr) { // key_name doesn't exist
        // insert in object file
        vector<char> vect(key_name.length());
	    copy(key_name.begin(), key_name.end(), vect.begin());
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
        value_id = config.get_object_file().write(*value);

        // insert in bpt
        bpt.insert(
            Record(hash_value[0], hash_value[1], value_id)
        );
    }
    else { // value already exists
        value_id = next->ids[2];
    }

    u_int64_t mask = element.is_node() ? NODE_MASK : EDGE_MASK;
    element2prop->insert(
        Record(
            element.get_id()|mask,
            key_id|KEY_MASK,
            value_id|VALUE_MASK
        )
    );
    prop2element->insert(
        Record(
            key_id|KEY_MASK,
            value_id|VALUE_MASK,
            element.get_id()|mask
        )
    );
}

Node RelationalGraph::get_node(u_int64_t id)
{
    return Node(id & UNMASK);
}

Label RelationalGraph::get_label(u_int64_t id)
{
    auto bytes = config.get_object_file().read(id&UNMASK);
    string label_name(bytes->begin(), bytes->end());
    return Label(label_name);
}

Key RelationalGraph::get_key(u_int64_t id)
{
    auto bytes = config.get_object_file().read(id&UNMASK);
    string key_name(bytes->begin(), bytes->end());
    return Key(key_name);
}

unique_ptr<Value> RelationalGraph::get_value(u_int64_t id)
{
    auto bytes = config.get_object_file().read(id&UNMASK);
    string str(bytes->begin(), bytes->end());
    return make_unique<ValueString>(str);
}

ObjectId RelationalGraph::get_label_id(Label const& label)
{
    u_int64_t hash[2]; // check MD5_DIGEST_LENGTH == 16?
    string label_name = label.get_label_name();
    MD5((const unsigned char*)label_name.c_str(), label_name.size(), (unsigned char *)hash);

    u_int64_t label_id;

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
    u_int64_t hash[2]; // check MD5_DIGEST_LENGTH == 16?
    string key_name = key.get_key_name();
    MD5((const unsigned char*)key_name.c_str(), key_name.size(), (unsigned char *)hash);

    u_int64_t key_id;

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
    u_int64_t hash[2]; // check MD5_DIGEST_LENGTH == 16?
    auto bytes = value.get_bytes();
    MD5((const unsigned char*)bytes->data(), bytes->size(), (unsigned char *)hash);

    u_int64_t value_id;

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
