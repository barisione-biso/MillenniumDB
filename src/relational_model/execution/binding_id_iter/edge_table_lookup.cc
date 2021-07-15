#include "edge_table_lookup.h"

#include <cassert>

#include "relational_model/models/quad_model/quad_model.h"

EdgeTableLookup::EdgeTableLookup(RandomAccessTable<3>& table, VarId edge, Id from, Id to, Id type) :
    table (table),
    edge  (edge),
    from  (from),
    to    (to),
    type  (type) { }


void EdgeTableLookup::analyze(int indent) const {
    for (int i = 0; i < indent; ++i) {
        std::cout << ' ';
    }
    std::cout << "EdgeTableLookup(lookups: " << lookups << ", found: " << results << ")\n";
}


void EdgeTableLookup::begin(BindingId& parent_binding) {
    this->parent_binding = &parent_binding;
    already_looked = false;
}


bool EdgeTableLookup::next() {
    if (already_looked) {
        return false;
    } else {
        already_looked = true;
        ++lookups;

        auto edge_assignation = (*parent_binding)[edge];
        if ( (QuadModel::TYPE_MASK & edge_assignation.id) != QuadModel::CONNECTION_MASK) {
            return false;
        }
        auto edge_id = QuadModel::VALUE_MASK & edge_assignation.id;
        assert(edge_id > 0);

        auto record = table[edge_id - 1]; // first edge has the id 1, and its inserted at pos 0 in the table

        // if mask was an edge this should not be nullptr
        assert(record != nullptr);

        auto check_id = [] (BindingId& binding, Id id, ObjectId obj_id) -> bool {
            if (std::holds_alternative<VarId>(id)) {
                auto binding_value = binding[std::get<VarId>(id)];
                if (!binding_value.is_null() && binding_value != obj_id) {
                    return false;
                } else {
                    binding.add(std::get<VarId>(id), obj_id);
                }
            } else { // std::holds_alternative<ObjectId>(id)
                auto value = std::get<ObjectId>(id);
                if (!value.is_null() && value != obj_id) {
                    return false;
                }
            }
            return true;
        };

        // check if assignated variables (not null) have the same value
        if (   check_id(*parent_binding, from, ObjectId(record->ids[0]))
            && check_id(*parent_binding, to,   ObjectId(record->ids[1]))
            && check_id(*parent_binding, type, ObjectId(record->ids[2])))
        {
            ++results;
            return true;
        } else {
            return false;
        }
    }
}


void EdgeTableLookup::reset() {
    already_looked = false;
}


void EdgeTableLookup::assign_nulls() {
    parent_binding->add(edge, ObjectId::get_null());
    if (std::holds_alternative<VarId>(from)) {
        parent_binding->add(std::get<VarId>(from), ObjectId::get_null());
    }
    if (std::holds_alternative<VarId>(to)) {
        parent_binding->add(std::get<VarId>(to), ObjectId::get_null());
    }
    if (std::holds_alternative<VarId>(type)) {
        parent_binding->add(std::get<VarId>(type), ObjectId::get_null());
    }
}
