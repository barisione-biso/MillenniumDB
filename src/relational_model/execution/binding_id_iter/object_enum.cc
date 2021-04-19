#include "object_enum.h"

using namespace std;

ObjectEnum::ObjectEnum(std::size_t /*binding_size*/, VarId var_id, const uint64_t mask, const uint64_t max_count) :
    var_id    (var_id),
    mask      (mask),
    max_count (max_count) { }


void ObjectEnum::begin(BindingId& parent_binding, bool parent_has_next) {
    this->parent_binding = &parent_binding;
    if (parent_has_next) {
        current_node = 0;
    }
    else {
        current_node = max_count;
    }
}


bool ObjectEnum::next() {
    if (current_node < max_count) {
        current_node++;
        parent_binding->add(var_id, ObjectId(mask | current_node));
        results++;
        return true;
    } else {
        return false;
    }
}


void ObjectEnum::reset() {
    current_node = 0;
}


void ObjectEnum::assign_nulls() {
    parent_binding->add(var_id, ObjectId::get_null());
}


void ObjectEnum::analyze(int indent) const {
    for (int i = 0; i < indent; ++i) {
        cout << ' ';
    }
    cout << "ObjectEnum(results: " << results << ")\n";
}
