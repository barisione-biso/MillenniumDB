#include "binding_order_by.h"

#include <cassert>
#include <iostream>

#include "base/binding/binding.h"

using namespace std;


BindingOrderBy::BindingOrderBy(GraphModel& model, vector<pair<string, VarId>> order_vars, Binding& child_binding, size_t binding_size) :
    order_vars    (move(order_vars)),
    model         (model),
    binding_size  (binding_size),
    child_binding (child_binding)

{
    binding_id = new ObjectId(binding_size);
}


BindingOrderBy::~BindingOrderBy() {
    delete binding_id;
}


GraphObject BindingOrderBy::operator[](const VarId var) {
    assert(var.id < binding_size);
    return model.get_graph_object(binding_id[var.id]);
}


void BindingOrderBy::update_binding_object(std::vector<uint64_t> obj_ids) {
    assert(obj_ids.size() == binding_size);
    for (size_t i = 0; i < binding_size; i++) {
        binding_id[i] = ObjectId(obj_ids[i]);
    }
}
