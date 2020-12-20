#include "binding_order_by.h"

#include <cassert>
#include <iostream>

#include "base/binding/binding.h"
#include "storage/tuple_collection/tuple_collection.h"

using namespace std;


BindingOrderBy::BindingOrderBy(GraphModel& model, vector<pair<string, VarId>> order_vars, Binding& child_binding, size_t binding_size) :
    order_vars    (move(order_vars)),
    model         (model),
    binding_size  (binding_size),
    child_binding (child_binding)

{
}


BindingOrderBy::~BindingOrderBy() = default;

std::ostream& BindingOrderBy::print_to_ostream(std::ostream& os) const {
    return os;
}

void BindingOrderBy::finish_read_of_child() {
    consume_from_child = false;
}

GraphObject BindingOrderBy::operator[](const VarId var) {
    assert(var.id < binding_size);
    if (consume_from_child) {
        return child_binding[var];
    } else {
      return objects_vector[var.id];
    }
}


void BindingOrderBy::update_binding_object(std::vector<GraphObject> graph_obj) {
    objects_vector = move(graph_obj);
}
