#include "quad_model.h"

#include <new>

#include "base/graph/anonymous_node.h"
#include "base/graph/identifiable_node.h"
#include "base/graph/edge.h"
#include "base/graph/value/value_int.h"
#include "base/graph/value/value_bool.h"
#include "base/graph/value/value_float.h"
#include "base/graph/value/value_string.h"
#include "relational_model/models/quad_model/query_optimizer/query_optimizer.h"
#include "storage/buffer_manager.h"
#include "storage/file_manager.h"

using namespace std;

QuadModel::QuadModel(const std::string& db_folder, const int buffer_pool_size)
    // catalog       (reinterpret_cast<Catalog&>(catalog_buf)),
    // object_file   (reinterpret_cast<ObjectFile&>(object_file_buf)),
    // strings_hash  (reinterpret_cast<ExtendibleHash&>(strings_cache_buf))
{
    FileManager::init(db_folder);
    BufferManager::init(buffer_pool_size);

    new (&catalog())       QuadCatalog("catalog.dat");                    // placement new
    new (&object_file())   ObjectFile("object_file.dat");                 // placement new
    new (&strings_hash())  ExtendibleHash(object_file(), "str_hash.dat"); // placement new
    // object_file   = ObjectFile("object_file.dat");
    // strings_hash  = ExtendibleHash(object_file, "str_hash.dat");
    // catalog       = Catalog("catalog.dat");
    // strings_cache = make_unique<StringsCache>(1000);

    node_table = make_unique<RandomAccessTable<1>>("nodes.table");
    edge_table = make_unique<RandomAccessTable<3>>("edges.table");

    // Create BPT
    label_node = make_unique<BPlusTree<2>>("label_node");
    node_label = make_unique<BPlusTree<2>>("node_label");

    object_key_value = make_unique<BPlusTree<3>>("object_key_value");
    key_value_object = make_unique<BPlusTree<3>>("key_value_object");

    from_to_type_edge = make_unique<BPlusTree<4>>("from_to_type_edge");
    to_type_from_edge = make_unique<BPlusTree<4>>("to_type_from_edge");
    type_from_to_edge = make_unique<BPlusTree<4>>("type_from_to_edge");

    catalog().print();
}


QuadModel::~QuadModel() {
    strings_hash().~ExtendibleHash();
    object_file().~ObjectFile();
    catalog().~QuadCatalog();

    label_node.reset();
    node_label.reset();

    object_key_value.reset();
    key_value_object.reset();

    from_to_type_edge.reset();
    to_type_from_edge.reset();
    type_from_to_edge.reset();

    buffer_manager.~BufferManager();
    file_manager.~FileManager();
}


std::unique_ptr<BindingIter> QuadModel::exec(OpSelect& op_select) {
    auto query_optimizer = QueryOptimizer(*this);
    return query_optimizer.exec(op_select);
}


std::unique_ptr<BindingIter> QuadModel::exec(manual_plan_ast::Root&) {
    // TODO:
    return nullptr;
}


uint64_t QuadModel::get_external_id(const string& str, bool create_if_not_exists) {
    return strings_hash().get_id(str, create_if_not_exists);
}


ObjectId QuadModel::get_string_id(const string& str, bool create_if_not_exists) {
    int string_len = str.length();

    if (string_len > MAX_INLINED_BYTES) {
        auto external_id =  get_external_id(str, create_if_not_exists);
        if (external_id == ObjectId::OBJECT_ID_NOT_FOUND) {
            return ObjectId(external_id);
        } else {
            return ObjectId(external_id | VALUE_EXTERNAL_STR_MASK);
        }
    } else {
        uint64_t res = 0;
        int shift_size = 0;
        for (uint64_t byte : str) { // MUST convert to 64bits or shift (shift_size >=32) is undefined behaviour
            res |= byte << shift_size;
            shift_size += 8;
        }
        return ObjectId(res | VALUE_INLINE_STR_MASK);
    }
}


ObjectId QuadModel::get_identifiable_object_id(const string& str, bool create_if_not_exists) {
    int string_len = str.length();

    if (string_len > MAX_INLINED_BYTES) {
        auto external_id =  get_external_id(str, create_if_not_exists);
        if (external_id == ObjectId::OBJECT_ID_NOT_FOUND) {
            return ObjectId(external_id);
        } else {
            return ObjectId(external_id | IDENTIFIABLE_NODE_MASK);
        }
    } else {
        uint64_t res = 0;
        int shift_size = 0;
        for (uint64_t byte : str) { // MUST convert to 64bits or shift (shift_size >=32) is undefined behaviour
            res |= byte << shift_size;
            shift_size += 8;
        }
        return ObjectId(res | INLINED_ID_NODE_MASK);
    }
}

ObjectId QuadModel::get_value_id(const Value& value, bool create_if_not_exists) {
    switch (value.type()) {
        case ObjectType::value_string : {
            const auto& string_value = static_cast<const ValueString&>(value);
            return get_string_id(string_value.value, create_if_not_exists);
        }

        case ObjectType::value_int : {
            const auto& int_value = static_cast<const ValueInt&>(value);
            auto value = int_value.value;

            uint64_t mask = VALUE_POSITIVE_INT_MASK;
            if (value < 0) {
                value *= -1;
                mask = VALUE_NEGATIVE_INT_MASK;
            }

            // check if it needs more than 7 bytes
            if ( (value & 0xFF00'0000'0000'0000UL) == 0) {
                return ObjectId(mask | value);
            } else {
                // VALUE_EXTERNAL_INT_MASK
                throw std::logic_error("NOT SUPPORTED YET");
            }
        }

        case ObjectType::value_float : {
            const auto& value_float = static_cast<const ValueFloat&>(value);
            auto bytes = make_unique<vector<unsigned char>>(sizeof(value_float.value));
            memcpy(bytes->data(), &value_float.value, sizeof(value_float.value));

            uint64_t res = 0;
            int shift_size = 0;
            for (uint64_t byte : *bytes) {
                res |= byte << shift_size;
                shift_size += 8;
            }
            return ObjectId(VALUE_FLOAT_MASK | res);
        }

        case ObjectType::value_bool : {
            const auto& value_bool = static_cast<const ValueBool&>(value);
            if (value_bool.value) {
                return ObjectId(VALUE_BOOL_MASK | 0x01);
            } else {
                return ObjectId(VALUE_BOOL_MASK | 0x00);
            }
        }

        default : {
            throw logic_error("Unexpected value type.");
        }
    }
}


shared_ptr<GraphObject> QuadModel::get_graph_object(ObjectId object_id) {
    // TODO:
    if (object_id.not_found()) {
        return make_shared<ValueString>("");
    }
    auto mask = object_id.id & TYPE_MASK;
    auto unmasked_id = object_id.id & VALUE_MASK;
    switch (mask) {
        case VALUE_EXTERNAL_STR_MASK : {
            // TODO: measure performance of using strings cache
            // auto cached_string = strings_cache->get(unmasked_id);
            // if (cached_string != nullptr) {
                // return cached_string;
            // } else {
                auto bytes = object_file().read(unmasked_id);
                string value_string(bytes->begin(), bytes->end());
                // strings_cache->insert(unmasked_id, value_string);
                return make_shared<ValueString>(move(value_string));
            // }
        }

        case VALUE_INLINE_STR_MASK : {
            string value_string = "";
            int shift_size = 0;
            for (int i = 0; i < MAX_INLINED_BYTES; i++) {
                uint8_t byte = (object_id.id >> shift_size) & 0xFF;
                if (byte == 0x00) {
                    break;
                }
                value_string.push_back(byte);
                shift_size += 8;
            }
            return make_shared<ValueString>(move(value_string));
        }

        case VALUE_POSITIVE_INT_MASK : {
            static_assert(sizeof(int64_t) == 8, "int64_t must be 8 bytes");
            int64_t i = object_id.id & 0x00FF'FFFF'FFFF'FFFFUL;
            return make_shared<ValueInt>(i);
        }

        case VALUE_NEGATIVE_INT_MASK : {
            static_assert(sizeof(int64_t) == 8, "int64_t must be 8 bytes");
            int64_t i = object_id.id & 0x00FF'FFFF'FFFF'FFFFUL;
            return make_shared<ValueInt>(i*-1);
        }

        case VALUE_FLOAT_MASK : {
            static_assert(sizeof(float) == 4, "float must be 4 bytes");
            float f;
            uint8_t* dest = (uint8_t*)&f;
            dest[0] =  object_id.id        & 0xFF;
            dest[1] = (object_id.id >> 8)  & 0xFF;
            dest[2] = (object_id.id >> 16) & 0xFF;
            dest[3] = (object_id.id >> 24) & 0xFF;
            return make_shared<ValueFloat>(f);
        }

        case VALUE_BOOL_MASK : {
            bool b;
            uint8_t* dest = (uint8_t*)&b;
            *dest = object_id.id & 0xFF;
            return make_shared<ValueBool>(b);
        }

        case IDENTIFIABLE_NODE_MASK : {
            // TODO: measure performance of using strings cache
            // auto cached_string = strings_cache->get(unmasked_id);
            // if (cached_string != nullptr) {
                // return cached_string;
            // } else {
                auto bytes = object_file().read(unmasked_id);
                string value_string(bytes->begin(), bytes->end());
                // string value_string = std::to_string(unmasked_id);
                // strings_cache->insert(unmasked_id, value_string);
                return make_shared<IdentifiableNode>(move(value_string));
            // }
        }

        case INLINED_ID_NODE_MASK : {
            string value_string = "";
            int shift_size = 0;
            for (int i = 0; i < MAX_INLINED_BYTES; i++) {
                uint8_t byte = (object_id.id >> shift_size) & 0xFF;
                if (byte == 0x00) {
                    break;
                }
                value_string.push_back(byte);
                shift_size += 8;
            }
            return make_shared<IdentifiableNode>(move(value_string));
        }

        case ANONYMOUS_NODE_MASK : {
            return make_shared<AnonymousNode>(unmasked_id);
        }

        case CONNECTION_MASK : {
            return make_shared<Edge>(unmasked_id);
        }

        default : {
            cout << "wrong value prefix:\n";
            printf("  obj_id: %lX\n", object_id.id);
            printf("  mask: %lX\n", mask);
            string value_string = "";
            return make_shared<ValueString>(move(value_string));
        }
    }
}
