#include "edge_table_lookup.h"

#include <cassert>

#include "relational_model/models/quad_model/quad_model.h"

EdgeTableLookup::EdgeTableLookup(std::size_t binding_size, RandomAccessTable<3>& table, VarId edge, Id from, Id to, Id type) :
    BindingIdIter(binding_size),
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


BindingId& EdgeTableLookup::begin(BindingId& input) {
    already_looked = false;
    my_input = &input;
    return my_binding;
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

        auto edge_assignation = (*my_input)[edge];
        auto edge_id = QuadModel::VALUE_MASK & edge_assignation.id;

        assert(edge_id > 0);
        auto record = table[edge_id - 1]; // first edge has the id 1, and its inserted at pos 0 in the table
        if (record == nullptr) {
            return false;
        } else {
            my_binding.add_all(*my_input);

            // check if assignated variables (not null) have the same value
            if (std::holds_alternative<VarId>(from)) {
                auto from_value = (*my_input)[std::get<VarId>(from)];
                if (!from_value.is_null() && from_value.id != record->ids[0]) {
                    return false;
                } else {
                    my_binding.add(std::get<VarId>(from), ObjectId(record->ids[0]));
                }
            } else { // std::holds_alternative<ObjectId>(from)
                auto from_value = std::get<ObjectId>(from);
                if (!from_value.is_null() && from_value.id != record->ids[0]) {
                    return false;
                } else {
                    my_binding.add(std::get<VarId>(from), ObjectId(record->ids[0]));
                }
            }
            if (std::holds_alternative<VarId>(to)) {
                auto to_value = (*my_input)[std::get<VarId>(to)];
                if (!to_value.is_null() && to_value.id != record->ids[1]) {
                    return false;
                } else {
                    my_binding.add(std::get<VarId>(to), ObjectId(record->ids[1]));
                }
            } else { // std::holds_alternative<ObjectId>(to)
                auto to_value = std::get<ObjectId>(to);
                if (!to_value.is_null() && to_value.id != record->ids[1]) {
                    return false;
                } else {
                    my_binding.add(std::get<VarId>(to), ObjectId(record->ids[1]));
                }
            }
            if (std::holds_alternative<VarId>(type)) {
                auto type_value = (*my_input)[std::get<VarId>(type)];
                if (!type_value.is_null() && type_value.id != record->ids[2]) {
                    return false;
                } else {
                    my_binding.add(std::get<VarId>(type), ObjectId(record->ids[2]));
                }
            } else { // std::holds_alternative<ObjectId>(type)
                auto type_value = std::get<ObjectId>(type);
                if (!type_value.is_null() && type_value.id != record->ids[2]) {
                    return false;
                } else {
                    my_binding.add(std::get<VarId>(type), ObjectId(record->ids[2]));
                }
            }
            ++results;
            return true;
        }
    }
}
