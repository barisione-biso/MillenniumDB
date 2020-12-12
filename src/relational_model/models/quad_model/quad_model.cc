#include "quad_model.h"

#include <new>

#include "base/graph/anonymous_node.h"
#include "base/graph/edge.h"
#include "relational_model/models/quad_model/graph_object_visitor.h"
#include "relational_model/models/quad_model/query_optimizer/query_optimizer.h"
#include "storage/buffer_manager.h"
#include "storage/file_manager.h"

using namespace std;

QuadModel::QuadModel(const std::string& db_folder, const int buffer_pool_size) {
    FileManager::init(db_folder);
    BufferManager::init(buffer_pool_size);

    new (&catalog())       QuadCatalog("catalog.dat");                    // placement new
    new (&object_file())   ObjectFile("object_file.dat");                 // placement new
    new (&strings_hash())  ExtendibleHash(object_file(), "str_hash.dat"); // placement new

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

    equal_from_to      = make_unique<BPlusTree<3>>("equal_from_to");
    equal_from_type    = make_unique<BPlusTree<3>>("equal_from_type");
    equal_to_type      = make_unique<BPlusTree<3>>("equal_to_type");
    equal_from_to_type = make_unique<BPlusTree<2>>("equal_from_to_type");

    equal_from_to_inverted   = make_unique<BPlusTree<3>>("equal_from_to_inverted");
    equal_from_type_inverted = make_unique<BPlusTree<3>>("equal_from_type_inverted");
    equal_to_type_inverted   = make_unique<BPlusTree<3>>("equal_to_type_inverted");

    catalog().print();
}


QuadModel::~QuadModel() {
    // Must destroy everything before buffer and file manager
    strings_hash().~ExtendibleHash();
    object_file().~ObjectFile();
    catalog().~QuadCatalog();

    node_table.reset();
    edge_table.reset();

    label_node.reset();
    node_label.reset();

    object_key_value.reset();
    key_value_object.reset();

    from_to_type_edge.reset();
    to_type_from_edge.reset();
    type_from_to_edge.reset();

    equal_from_to.reset();
    equal_from_type.reset();
    equal_to_type.reset();
    equal_from_to_type.reset();

    equal_from_to_inverted.reset();
    equal_from_type_inverted.reset();
    equal_to_type_inverted.reset();

    buffer_manager.~BufferManager();
    file_manager.~FileManager();
}


std::unique_ptr<BindingIter> QuadModel::exec(OpSelect& op_select) {
    auto query_optimizer = QueryOptimizer(*this);
    return query_optimizer.exec(op_select);
}


std::unique_ptr<BindingIter> QuadModel::exec(manual_plan::ast::ManualRoot& manual_plan) {
    auto query_optimizer = QueryOptimizer(*this);
    return query_optimizer.exec(manual_plan);
}


uint64_t QuadModel::get_external_id(const string& str, bool create_if_not_exists) {
    return strings_hash().get_id(str, create_if_not_exists);
}


ObjectId QuadModel::get_string_id(const string& str, bool create_if_not_exists) {
    return this->get_object_id(GraphObject::make_string(str), create_if_not_exists);
}


ObjectId QuadModel::get_identifiable_object_id(const string& str, bool create_if_not_exists) {
    return this->get_object_id(GraphObject::make_identifiable(str), create_if_not_exists);
}


GraphObject QuadModel::get_graph_object(ObjectId object_id) {
    if ( object_id.is_not_found() ) {
        return GraphObject::make_not_found();
    }
    if ( object_id.is_null() ) {
        return GraphObject::make_null();
    }
    auto mask        = object_id.id & TYPE_MASK;
    auto unmasked_id = object_id.id & VALUE_MASK;
    switch (mask) {
        case VALUE_EXTERNAL_STR_MASK : {
            const char* str = object_file().read(unmasked_id);
            return GraphObject::make_string_external(str);
        }

        case VALUE_INLINE_STR_MASK : {
            char c[8];
            int shift_size = 0;
            for (int i = 0; i < MAX_INLINED_BYTES; i++) {
                uint8_t byte = (object_id.id >> shift_size) & 0xFF;
                c[i] = byte;
                shift_size += 8;
            }
            c[7] = '\0';
            return GraphObject::make_string_inlined(c);
        }

        case VALUE_POSITIVE_INT_MASK : {
            static_assert(sizeof(int64_t) == 8, "int64_t must be 8 bytes");
            int64_t i = object_id.id & 0x00FF'FFFF'FFFF'FFFFUL;
            return GraphObject::make_int(i);
        }

        case VALUE_NEGATIVE_INT_MASK : {
            static_assert(sizeof(int64_t) == 8, "int64_t must be 8 bytes");
            int64_t i = object_id.id & 0x00FF'FFFF'FFFF'FFFFUL;
            return GraphObject::make_int(i*-1);
        }

        case VALUE_FLOAT_MASK : {
            static_assert(sizeof(float) == 4, "float must be 4 bytes");
            float f;
            uint8_t* dest = (uint8_t*)&f;
            dest[0] =  object_id.id        & 0xFF;
            dest[1] = (object_id.id >> 8)  & 0xFF;
            dest[2] = (object_id.id >> 16) & 0xFF;
            dest[3] = (object_id.id >> 24) & 0xFF;
            return GraphObject::make_float(f);
        }

        case VALUE_BOOL_MASK : {
            bool b;
            uint8_t* dest = (uint8_t*)&b;
            *dest = object_id.id & 0xFF;
            return GraphObject::make_bool(b);
        }

        case IDENTIFIABLE_EXTERNAL_MASK : {
            const char* str = object_file().read(unmasked_id);
            return GraphObject::make_identifiable_external(str);
        }

        case IDENTIFIABLE_INLINED_MASK : {
            char c[8];
            int shift_size = 0;
            for (int i = 0; i < MAX_INLINED_BYTES; i++) {
                uint8_t byte = (object_id.id >> shift_size) & 0xFF;
                c[i] = byte;
                shift_size += 8;
            }
            c[7] = '\0';
            return GraphObject::make_identifiable_inlined(c);
        }

        case ANONYMOUS_NODE_MASK : {
            return GraphObject::make_anonymous(unmasked_id);
        }

        case CONNECTION_MASK : {
            return GraphObject::make_edge(unmasked_id);
        }

        default : {
            cout << "wrong value prefix:\n";
            printf("  obj_id: %lX\n", object_id.id);
            printf("  mask:   %lX\n", mask);
            return GraphObject::make_null();
        }
    }
}


GraphObject QuadModel::get_property_value(GraphObject& var, const ObjectId key) {
    auto obj_id = get_object_id(var);

    auto it = object_key_value->get_range(
        RecordFactory::get(obj_id.id, key.id, 0),
        RecordFactory::get(obj_id.id, key.id, UINT64_MAX)
    );

    auto res = it->next();
    if (res != nullptr) {
        auto value_id = ObjectId(res->ids[2]);
        return get_graph_object(value_id);
    } else {
        return GraphObject::make_null();
    }
}


ObjectId QuadModel::get_object_id(const GraphObject& graph_object) {
    return get_object_id(graph_object, false);
}


ObjectId QuadModel::get_object_id(const GraphObject& graph_object, bool create_if_not_exists) {
    return std::visit(GraphObjectVisitor{ *this, create_if_not_exists }, graph_object.value);
}
