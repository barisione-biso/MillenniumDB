#include "edge_table_lookup.h"

#include "relational_model/models/quad_model/quad_model.h"

EdgeTableLookup::EdgeTableLookup(std::size_t binding_size, RandomAccessTable<3>& table, VarId edge, Id from, Id to, Id type) :
    // BindingIdIter(binding_size),
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


void EdgeTableLookup::begin(BindingId& parent_binding, bool) {
    already_looked = false;
    this->parent_binding = &parent_binding;
    // return my_binding;
}


void EdgeTableLookup::reset() {
    already_looked = false;
}


bool EdgeTableLookup::next() {
    if (already_looked) {
        return false;
    } else {
        already_looked = true;
        ++lookups;

        auto edge_assignation = (*parent_binding)[edge];
        auto edge_id = QuadModel::VALUE_MASK & edge_assignation.id;

        auto record = table[edge_id];
        if (record == nullptr) {
            return false;
        } else {
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
            // my_input.add_all(*my_input);
            ++results;
            return true;
        }
    }
}
