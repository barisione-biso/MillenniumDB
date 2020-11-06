#include "object_enum.h"

using namespace std;

ObjectEnum::ObjectEnum(std::size_t binding_size, VarId var_id, const uint64_t mask, const uint64_t max_count) :
    BindingIdIter(binding_size),
    var_id    (var_id),
    mask      (mask),
    max_count (max_count) { }


BindingId& ObjectEnum::begin(BindingId& input) {
    my_input = &input;
    current_node = 0;
    return my_binding;
}


bool ObjectEnum::next() {
    ++current_node;
    if (current_node <= max_count) {
        my_binding.add_all(*my_input);
        my_binding.add(var_id, ObjectId(mask | current_node));
        return true;
    } else {
        return false;
    }
}


void ObjectEnum::reset() {
    current_node = 0;
}


void ObjectEnum::analyze(int indent) const {
    for (int i = 0; i < indent; ++i) {
        cout << ' ';
    }
    cout << "ObjectEnum(found: " << current_node << ")\n";
}
