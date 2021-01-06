#include "quad_model.h"

#include <new>

#include "base/graph/anonymous_node.h"
#include "base/graph/edge.h"
#include "relational_model/models/quad_model/graph_object_visitor.h"
#include "relational_model/models/quad_model/query_optimizer/query_optimizer.h"
#include "relational_model/models/quad_model/query_optimizer/binding_iter_visitor.h"
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
    auto query_optimizer = BindingIterVisitor(*this, op_select.get_var_names());
    return query_optimizer.exec(op_select);
}


std::unique_ptr<BindingIter> QuadModel::exec(manual_plan::ast::ManualRoot& manual_plan) {
    auto query_optimizer = QueryOptimizer(*this);
    return query_optimizer.exec(manual_plan);
}


uint64_t QuadModel::get_external_id(const string& str) {
    return strings_hash().get_id(str);
}


uint64_t QuadModel::get_or_create_external_id(const string& str, bool* created) {
    return strings_hash().get_or_create_id(str, created);
}


uint64_t QuadModel::get_or_create_identifiable_object_id(const std::string& str, bool* created) {
    if (str.size() < 8) {
        uint64_t res = 0;
        int shift_size = 0;
        // MUST convert to uint8_t and then to uint64_t.
        // Shift with shift_size >=32 is undefined behaviour.
        for (uint8_t byte : str) {
            uint64_t byte64 = static_cast<uint64_t>(byte);
            res |= byte64 << shift_size;
            shift_size += 8;
        }
        return res | GraphModel::IDENTIFIABLE_INLINED_MASK;
    } else {
        auto external_id = get_or_create_external_id(str, created);
        return external_id | GraphModel::IDENTIFIABLE_EXTERNAL_MASK;
    }
}


uint64_t QuadModel::get_or_create_string_id(const std::string& str) {
    if (str.size() < 8) {
        uint64_t res = 0;
        int shift_size = 0;
        // MUST convert to uint8_t and then to uint64_t.
        // Shift with shift_size >=32 is undefined behaviour.
        for (uint8_t byte : str) {
            uint64_t byte64 = static_cast<uint64_t>(byte);
            res |= byte64 << shift_size;
            shift_size += 8;
        }
        return res | GraphModel::VALUE_INLINE_STR_MASK;
    } else {
        bool created;
        auto external_id = get_or_create_external_id(str, &created);
        return external_id | GraphModel::VALUE_EXTERNAL_STR_MASK;
    }
}


uint64_t QuadModel::get_or_create_value_id(const GraphObject& obj) {
    if (std::holds_alternative<StringExternal>(obj.value)) {
        const std::string str(std::get<StringExternal>(obj.value).id);
        bool created;
        auto external_id = get_or_create_external_id(str, &created);
        return external_id | GraphModel::VALUE_EXTERNAL_STR_MASK;
    } else {
        GraphObjectVisitor visitor(*this);
        if (std::holds_alternative<StringInlined>(obj.value)) {
            return visitor(std::get<StringInlined>(obj.value)).id;
        }
        else if (std::holds_alternative<int64_t>(obj.value)) {
            return visitor(std::get<int64_t>(obj.value)).id;
        }
        else if (std::holds_alternative<float>(obj.value)) {
            return visitor(std::get<float>(obj.value)).id;
        }
        else if (std::holds_alternative<bool>(obj.value)) {
            return visitor(std::get<bool>(obj.value)).id;
        }
        else {
            throw std::logic_error("QuadModel::get_or_create_value_id(): unknown value type");
        }
    }
}


ObjectId QuadModel::get_string_id(const string& str) {
    if (str.size() < 8) {
        uint64_t res = 0;
        int shift_size = 0;
        // MUST convert to uint8_t and then to uint64_t.
        // Shift with shift_size >=32 is undefined behaviour.
        for (uint8_t byte : str) {
            uint64_t byte64 = static_cast<uint64_t>(byte);
            res |= byte64 << shift_size;
            shift_size += 8;
        }
        return ObjectId(res | GraphModel::VALUE_INLINE_STR_MASK);
    } else {
        auto external_id = get_external_id(str);
        if (external_id == ObjectId::OBJECT_ID_NOT_FOUND) {
            return ObjectId::get_not_found();
        } else {
            return ObjectId(external_id | GraphModel::VALUE_EXTERNAL_STR_MASK);
        }
    }
}


ObjectId QuadModel::get_identifiable_object_id(const string& str) {
    if (str.size() < 8) {
        uint64_t res = 0;
        int shift_size = 0;
        // MUST convert to uint8_t and then to uint64_t.
        // Shift with shift_size >=32 is undefined behaviour.
        for (uint8_t byte : str) {
            uint64_t byte64 = static_cast<uint64_t>(byte);
            res |= byte64 << shift_size;
            shift_size += 8;
        }
        return ObjectId(res | GraphModel::IDENTIFIABLE_INLINED_MASK);
    } else {
        auto external_id = get_external_id(str);
        if (external_id == ObjectId::OBJECT_ID_NOT_FOUND) {
            return ObjectId::get_not_found();
        } else {
            return ObjectId(external_id | GraphModel::IDENTIFIABLE_EXTERNAL_MASK);
        }
    }
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
    return std::visit(GraphObjectVisitor{ *this }, graph_object.value);
}
