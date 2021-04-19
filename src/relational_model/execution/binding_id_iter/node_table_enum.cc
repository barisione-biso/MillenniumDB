#include "node_table_enum.h"

NodeTableEnum::NodeTableEnum(std::size_t /*binding_size*/, const VarId var_id, RandomAccessTable<1>& table) :
    var_id (var_id),
    table  (table) { }


void NodeTableEnum::begin(BindingId& parent_binding, bool parent_has_next) {
    this->parent_binding = &parent_binding;
    if (parent_has_next) {
        current_pos = 0;
    }
    else {
        current_pos = -1;
    }
}


bool NodeTableEnum::next() {
    auto record = table[current_pos];
    if (record != nullptr) {
        current_pos++;
        parent_binding->add(var_id, ObjectId(record->ids[0]));
        ++results;
        return true;
    } else {
        return false;
    }
}


void NodeTableEnum::reset() {
    current_pos = 0;
}


void NodeTableEnum::assign_nulls() {
    parent_binding->add(var_id, ObjectId::get_null());
}


void NodeTableEnum::analyze(int indent) const {
    for (int i = 0; i < indent; ++i) {
        std::cout << ' ';
    }
    std::cout << "NodeTableEnum(found: " << results << ")\n";
}
