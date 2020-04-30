#include "relational_model.h"

#include <iostream>
#include <new>         // placement new
#include <type_traits> // aligned_storage

#include <openssl/md5.h>

#include "base/graph/edge.h"
#include "base/graph/node.h"
#include "base/graph/value/value_int.h"
#include "base/graph/value/value_bool.h"
#include "base/graph/value/value_float.h"
#include "base/graph/value/value_string.h"
#include "relational_model/graph/relational_graph.h"
#include "storage/catalog/catalog.h"

using namespace std;

// zero initialized at load time
static int nifty_counter;
// memory for the object
static typename std::aligned_storage<sizeof(RelationalModel), alignof(RelationalModel)>::type relational_model_buf;
// global object
RelationalModel& relational_model = reinterpret_cast<RelationalModel&>(relational_model_buf);


RelationalModel::RelationalModel() = default;
RelationalModel::~RelationalModel() = default;


void RelationalModel::init() {
    catalog.init();
    object_file = make_unique<ObjectFile>(object_file_name);

    // Create BPT Params
    auto bpt_params_hash2id = make_unique<BPlusTreeParams>(hash2id_name, 3); // Hash:2*64 + Key:64

    auto bpt_params_label2node = make_unique<BPlusTreeParams>(RelationalModel::label2node_name, 2);
    auto bpt_params_label2edge = make_unique<BPlusTreeParams>(RelationalModel::label2edge_name, 2);
    auto bpt_params_node2label = make_unique<BPlusTreeParams>(RelationalModel::node2label_name, 2);
    auto bpt_params_edge2label = make_unique<BPlusTreeParams>(RelationalModel::edge2label_name, 2);

    auto bpt_params_key_value_node = make_unique<BPlusTreeParams>(RelationalModel::key_value_node_name, 3);
    auto bpt_params_node_key_value = make_unique<BPlusTreeParams>(RelationalModel::node_key_value_name, 3);
    auto bpt_params_key_node_value = make_unique<BPlusTreeParams>(RelationalModel::key_node_value_name, 3);

    auto bpt_params_key_value_edge = make_unique<BPlusTreeParams>(RelationalModel::key_value_edge_name, 3);
    auto bpt_params_edge_key_value = make_unique<BPlusTreeParams>(RelationalModel::edge_key_value_name, 3);
    auto bpt_params_key_edge_value = make_unique<BPlusTreeParams>(RelationalModel::key_edge_value_name, 3);

    auto bpt_params_from_to_edge = make_unique<BPlusTreeParams>(RelationalModel::from_to_edge_name, 3);
    auto bpt_params_to_edge_from = make_unique<BPlusTreeParams>(RelationalModel::to_edge_from_name, 3);
    auto bpt_params_edge_from_to = make_unique<BPlusTreeParams>(RelationalModel::edge_from_to_name, 3);

    // Create BPT
    hash2id = make_unique<BPlusTree>(move(bpt_params_hash2id));

    label2node = make_unique<BPlusTree>(move(bpt_params_label2node));
    label2edge = make_unique<BPlusTree>(move(bpt_params_label2edge));
    node2label = make_unique<BPlusTree>(move(bpt_params_node2label));
    edge2label = make_unique<BPlusTree>(move(bpt_params_edge2label));

    key_value_node = make_unique<BPlusTree>(move(bpt_params_key_value_node));
    node_key_value = make_unique<BPlusTree>(move(bpt_params_node_key_value));
    key_node_value = make_unique<BPlusTree>(move(bpt_params_key_node_value));

    key_value_edge = make_unique<BPlusTree>(move(bpt_params_key_value_edge));
    edge_key_value = make_unique<BPlusTree>(move(bpt_params_edge_key_value));
    key_edge_value = make_unique<BPlusTree>(move(bpt_params_key_edge_value));

    from_to_edge = make_unique<BPlusTree>(move(bpt_params_from_to_edge));
    to_edge_from = make_unique<BPlusTree>(move(bpt_params_to_edge_from));
    edge_from_to = make_unique<BPlusTree>(move(bpt_params_edge_from_to));

    catalog.print();
}


uint64_t RelationalModel::get_external_id(std::unique_ptr< std::vector<unsigned char> > bytes) {
    static_assert(MD5_DIGEST_LENGTH == 16, "Hash is expected to use 16 bytes.");
    uint64_t hash[2];
    MD5((const unsigned char*)bytes->data(), bytes->size(), (unsigned char *)hash);

    // check if bpt contains object
    BPlusTree& bpt = get_hash2id_bpt();
    auto iter = bpt.get_range(
        Record(hash[0], hash[1], 0),
        Record(hash[0], hash[1], UINT64_MAX)
    );
    auto next = iter->next();
    if (next == nullptr) { // object doesn't exist
        return NOT_FOUND_OBJECT_ID;
    } else {               // object already exists
        return next->ids[2];
    }
}


uint64_t RelationalModel::get_or_create_external_id(std::unique_ptr< std::vector<unsigned char> > bytes) {
    static_assert(MD5_DIGEST_LENGTH == 16, "Hash is expected to use 16 bytes.");
    uint64_t hash[2];
    MD5((const unsigned char*)bytes->data(), bytes->size(), (unsigned char *)hash);

    // check if bpt contains object
    BPlusTree& hash2id = get_hash2id_bpt();
    auto iter = hash2id.get_range(
        Record(hash[0], hash[1], 0),
        Record(hash[0], hash[1], UINT64_MAX)
    );
    auto next = iter->next();
    if (next == nullptr) { // object doesn't exist
        // Insert in object file
        uint64_t obj_id = get_object_file().write(*bytes);
        // Insert in bpt
        hash2id.insert( Record(hash[0], hash[1], obj_id) );
        return obj_id;
    } else {               // object already exists
        return next->ids[2];
    }
}


ObjectId RelationalModel::get_string_unmasked_id(const string& str) {
    int string_len = str.length();

    if (string_len > MAX_INLINED_BYTES) {
        auto bytes = make_unique<vector<unsigned char>>(string_len);
        copy(str.begin(), str.end(), bytes->begin());

        return ObjectId( get_external_id(move(bytes)) );
    } else {
        uint64_t res = 0;
        int shift_size = 0;
        for (uint64_t byte : str) { // MUST convert to 64bits or shift (shift_size >=32) is undefined behaviour
            res |= byte << shift_size;
            shift_size += 8;
        }
        return ObjectId(res);
    }
}


ObjectId RelationalModel::get_value_masked_id(const Value& value) {
    auto obj_bytes = value.get_bytes();
    if (obj_bytes->size() > MAX_INLINED_BYTES) {
        return get_external_id(move(obj_bytes)) | get_value_mask(value);
    }
    else {
        uint64_t res = 0;
        int shift_size = 0;
        for (uint64_t byte : *obj_bytes) { // MUST convert to 64bits or shift (shift_size >=32) is undefined behaviour
            res |= byte << shift_size;
            shift_size += 8;
        }
        return ObjectId( res | get_value_mask(value) );
    }
}


ObjectId RelationalModel::get_or_create_string_unmasked_id(const std::string& str) {
    int string_len = str.length();

    if (string_len > MAX_INLINED_BYTES) {
        auto bytes = make_unique<vector<unsigned char>>(string_len);
        copy(str.begin(), str.end(), bytes->begin());

        return ObjectId( get_or_create_external_id(move(bytes)) );
    }
    else {
        uint64_t res = 0;
        int shift_size = 0;
        for (uint64_t byte : str) { // MUST convert to 64bits or shift (shift_size >=32) is undefined behaviour
            res |= byte << shift_size;
            shift_size += 8;
        }
        return ObjectId(res);
    }
}


ObjectId RelationalModel::get_or_create_value_masked_id(const Value& value) {
    auto obj_bytes = value.get_bytes();
    if (obj_bytes->size() > MAX_INLINED_BYTES) {
        return get_or_create_external_id(move(obj_bytes)) | get_value_mask(value);
    }
    else {
        uint64_t res = 0;
        int shift_size = 0;
        for (uint64_t byte : *obj_bytes) { // MUST convert to 64bits or shift (shift_size >=32) is undefined behaviour
            res |= byte << shift_size;
            shift_size += 8;
        }
        return ObjectId( res | get_value_mask(value) );
    }
}


shared_ptr<GraphObject> RelationalModel::get_graph_object(ObjectId object_id) {
    auto mask = object_id.id & TYPE_MASK;

    if (mask == VALUE_EXTERNAL_STR_MASK) {
        auto bytes = object_file->read(object_id & VALUE_MASK);
        string value_string(bytes->begin(), bytes->end());
        return make_shared<ValueString>(move(value_string));
    }
    else if (mask == VALUE_INLINE_STR_MASK) {
        string value_string = "";
        int shift_size = 0;
        for (int i = 0; i < MAX_INLINED_BYTES; i++) {
            uint8_t byte = (object_id >> shift_size) & 0xFF;
            if (byte == 0x00) {
                break;
            }
            value_string.push_back(byte);
            shift_size += 8;
        }
        return make_shared<ValueString>(move(value_string));
    }
    else if (mask == VALUE_INT_MASK) {
        static_assert(sizeof(int) == 4, "int must be 4 bytes");
        int i;
        uint8_t* dest = (uint8_t*)&i;
        dest[0] = object_id & 0xFF;
        dest[1] = (object_id >> 8) & 0xFF;
        dest[2] = (object_id >> 16) & 0xFF;
        dest[3] = (object_id >> 24) & 0xFF;
        return make_shared<ValueInt>(i);
    }
    else if (mask == VALUE_FLOAT_MASK) {
        static_assert(sizeof(float) == 4, "float must be 4 bytes");
        float f;
        uint8_t* dest = (uint8_t*)&f;
        dest[0] = object_id & 0xFF;
        dest[1] = (object_id >> 8) & 0xFF;
        dest[2] = (object_id >> 16) & 0xFF;
        dest[3] = (object_id >> 24) & 0xFF;
        return make_shared<ValueFloat>(f);
    }
    else if (mask == VALUE_BOOL_MASK) {
        bool b;
        uint8_t* dest = (uint8_t*)&b;
        *dest = object_id & 0xFF;
        return make_shared<ValueBool>(b);
    }
    else if (mask == NODE_MASK) {
        return make_shared<Node>(object_id);
    }
    else if (mask == EDGE_MASK) {
        return make_shared<Edge>(object_id);
    }
    else {
        cout << "wrong value prefix:\n";
        printf("  obj_id: %lX\n", object_id.id);
        printf("  mask: %lX\n", mask);
        string value_string = "";
        return make_shared<ValueString>(move(value_string));
    }
}


RelationalGraph& RelationalModel::create_graph(const std::string& graph_name) {
    auto graph_id = catalog.create_graph(graph_name);
    return get_graph(graph_id);
}


RelationalGraph& RelationalModel::get_graph(GraphId graph_id) {
    auto search = graphs.find(graph_id);
    if (search != graphs.end()) {
        return *search->second.get();
    } else {
        graphs.insert({ graph_id, make_unique<RelationalGraph>(graph_id) });
        return *graphs[graph_id].get();
    }
}


uint64_t RelationalModel::get_value_mask(const Value& value) {
    auto type = value.type();
    if (type == ObjectType::value_string) {
        const auto& string_value = static_cast<const ValueString&>(value);
        if (string_value.value.size() <= MAX_INLINED_BYTES) {
            return VALUE_INLINE_STR_MASK;
        }
        else return VALUE_EXTERNAL_STR_MASK;
    }
    else if (type == ObjectType::value_int) {
        return VALUE_INT_MASK;
    }
    else if (type == ObjectType::value_float) {
        return VALUE_FLOAT_MASK;
    }
    else if (type == ObjectType::value_bool) {
        return VALUE_BOOL_MASK;
    }
    else {
        throw logic_error("Unexpected value type.");
    }
}


ObjectFile& RelationalModel::get_object_file() { return *object_file; }
BPlusTree&  RelationalModel::get_hash2id_bpt() { return *hash2id; }

BPlusTree& RelationalModel::get_label2node() { return *label2node; }
BPlusTree& RelationalModel::get_label2edge() { return *label2edge; }
BPlusTree& RelationalModel::get_node2label() { return *node2label; }
BPlusTree& RelationalModel::get_edge2label() { return *edge2label; }

BPlusTree& RelationalModel::get_key_value_node() { return *key_value_node; }
BPlusTree& RelationalModel::get_node_key_value() { return *node_key_value; }
BPlusTree& RelationalModel::get_key_node_value() { return *key_node_value; }

BPlusTree& RelationalModel::get_key_value_edge() { return *key_value_edge; }
BPlusTree& RelationalModel::get_edge_key_value() { return *edge_key_value; }
BPlusTree& RelationalModel::get_key_edge_value() { return *key_edge_value; }

BPlusTree& RelationalModel::get_from_to_edge() { return *from_to_edge; }
BPlusTree& RelationalModel::get_to_edge_from() { return *to_edge_from; }
BPlusTree& RelationalModel::get_edge_from_to() { return *edge_from_to; }

// Nifty counter trick
RelationalModelInitializer::RelationalModelInitializer () {
    if (nifty_counter++ == 0) new (&relational_model) RelationalModel(); // placement new
}

RelationalModelInitializer::~RelationalModelInitializer () {
    if (--nifty_counter == 0) (&relational_model)->~RelationalModel();
}
