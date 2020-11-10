#include "node_table_enum.h"

NodeTableEnum::NodeTableEnum(std::size_t binding_size, const VarId var_id, RandomAccessTable<1>& table) :
    BindingIdIter(binding_size),
    var_id (var_id),
    table  (table) { }


void NodeTableEnum::analyze(int indent) const {
    for (int i = 0; i < indent; ++i) {
        std::cout << ' ';
    }
    std::cout << "NodeTableEnum(found: " << results << ")\n";
}


BindingId& NodeTableEnum::begin(BindingId& input) {
    my_input = &input;
    current_pos = 0;
    return my_binding;
}


void NodeTableEnum::reset() {
    current_pos = 0;
}


bool NodeTableEnum::next() {
    auto record = table[current_pos++];
    if (record != nullptr) {
        my_binding.add_all(*my_input);
        my_binding.add(var_id, ObjectId(record->ids[0]));
        ++results;
        return true;
    } else {
        return false;
    }
}