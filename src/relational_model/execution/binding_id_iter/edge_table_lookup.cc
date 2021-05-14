#include "edge_table_lookup.h"

#include <cassert>

#include "relational_model/models/quad_model/quad_model.h"

EdgeTableLookup::EdgeTableLookup(std::size_t, RandomAccessTable<3>& table, VarId edge, Id from, Id to, Id type) :
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


void EdgeTableLookup::begin(BindingId& parent_binding, bool parent_has_next) {
    this->parent_binding = &parent_binding;
    already_looked = !parent_has_next;
}


bool EdgeTableLookup::next() {
    if (already_looked) {
        return false;
    } else {
        already_looked = true;
        ++lookups;

        auto edge_assignation = (*parent_binding)[edge];
        if ( (QuadModel::VALUE_MASK & edge_assignation.id) != QuadModel::CONNECTION_MASK) {
            return false;
        }
        auto edge_id = QuadModel::VALUE_MASK & edge_assignation.id;
        assert(edge_id > 0);

        auto record = table[edge_id - 1]; // first edge has the id 1, and its inserted at pos 0 in the table

        // if mask was and edge this should not be nullptr
        assert(record != nullptr);

        // check if assignated variables (not null) have the same value
        if (std::holds_alternative<VarId>(from)) {
            auto from_value = (*parent_binding)[std::get<VarId>(from)];
            if (!from_value.is_null() && from_value.id != record->ids[0]) {
                return false;
            } else {
                parent_binding->add(std::get<VarId>(from), ObjectId(record->ids[0]));
            }
        } else { // std::holds_alternative<ObjectId>(from)
            auto from_value = std::get<ObjectId>(from);
            if (!from_value.is_null() && from_value.id != record->ids[0]) {
                return false;
            } else {
                parent_binding->add(std::get<VarId>(from), ObjectId(record->ids[0]));
            }
        }
        if (std::holds_alternative<VarId>(to)) {
            auto to_value = (*parent_binding)[std::get<VarId>(to)];
            if (!to_value.is_null() && to_value.id != record->ids[1]) {
                return false;
            } else {
                parent_binding->add(std::get<VarId>(to), ObjectId(record->ids[1]));
            }
        } else { // std::holds_alternative<ObjectId>(to)
            auto to_value = std::get<ObjectId>(to);
            if (!to_value.is_null() && to_value.id != record->ids[1]) {
                return false;
            } else {
                parent_binding->add(std::get<VarId>(to), ObjectId(record->ids[1]));
            }
        }
        if (std::holds_alternative<VarId>(type)) {
            auto type_value = (*parent_binding)[std::get<VarId>(type)];
            if (!type_value.is_null() && type_value.id != record->ids[2]) {
                return false;
            } else {
                parent_binding->add(std::get<VarId>(type), ObjectId(record->ids[2]));
            }
        } else { // std::holds_alternative<ObjectId>(type)
            auto type_value = std::get<ObjectId>(type);
            if (!type_value.is_null() && type_value.id != record->ids[2]) {
                return false;
            } else {
                parent_binding->add(std::get<VarId>(type), ObjectId(record->ids[2]));
            }
        }
        ++results;
        return true;
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
