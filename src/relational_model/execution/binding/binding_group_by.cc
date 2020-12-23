#include "binding_group_by.h"

#include <cassert>
#include <iostream>

#include "base/binding/binding.h"

using namespace std;


BindingGroupBy::BindingGroupBy(GraphModel& model, vector<pair<string, VarId>> _group_vars, Binding& child_binding, size_t binding_size) :
    group_vars    (move(_group_vars)),
    model         (model),
    binding_size  (binding_size),
    child_binding (child_binding)

{ }


BindingGroupBy::~BindingGroupBy() = default;

std::ostream& BindingGroupBy::print_to_ostream(std::ostream& os) const {
    return os;
}


GraphObject BindingGroupBy::operator[](const VarId var) {
    assert(var.id < binding_size);
    return objects_vector[var.id];
}
