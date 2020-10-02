#include "object_enum.h"

using namespace std;

ObjectEnum::ObjectEnum(VarId var_id, const uint64_t mask, const uint64_t max_count) :
    var_id    (var_id),
    mask      (mask),
    max_count (max_count) { }


void ObjectEnum::begin(BindingId& input) {
    my_binding = make_unique<BindingId>(input.var_count());
    my_input = &input;
    current_node = 0;
}


BindingId* ObjectEnum::next() {
    ++current_node;
    if (current_node <= max_count) {
        my_binding->add_all(*my_input);
        my_binding->add(var_id, ObjectId(mask | current_node));
        return my_binding.get();
    } else {
        return nullptr;
    }
}


void ObjectEnum::reset(BindingId& input) {
    begin(input);
}


void ObjectEnum::analyze(int indent) const {
    for (int i = 0; i < indent; ++i) {
        cout << ' ';
    }
    cout << "ObjectEnum(found: " << current_node << ")\n";
}
