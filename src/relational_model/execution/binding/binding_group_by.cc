#include "binding_group_by.h"

#include <cassert>

#include "base/binding/binding.h"

using namespace std;


BindingGroupBy::BindingGroupBy(GraphModel& model, vector<pair<string, VarId>> _group_vars, Binding& child_binding, size_t binding_size) :
    group_vars    (_group_vars),
    model         (model),
    binding_size  (binding_size),
    child_binding (child_binding) { }


std::ostream& BindingGroupBy::print_to_ostream(std::ostream& os) const {
    return os;
}


GraphObject BindingGroupBy::operator[](const VarId var) {
    assert(var.id < binding_size);
    return current_objects[var.id];
}


void BindingGroupBy::update_binding(std::vector<GraphObject> new_tuple) {
    current_objects = move(new_tuple);
}
