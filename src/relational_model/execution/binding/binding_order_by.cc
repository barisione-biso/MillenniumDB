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
    binding_id = new ObjectId(binding_size);
}


BindingOrderBy::~BindingOrderBy() {
    delete binding_id;
}

std::ostream& BindingOrderBy::print_to_ostream(std::ostream& os) const {
    return os;
}



GraphObject BindingOrderBy::operator[](const VarId var) {
    assert(var.id < binding_size);
    return model.get_graph_object(binding_id[var.id]);
}


void BindingOrderBy::update_binding_object(uint8_t* graph_obj) {
    for (size_t i = 0; i < binding_size; i++) {
        const GraphObject* obj = reinterpret_cast<GraphObject*>(&graph_obj[i * TupleCollection::GRAPH_OBJECT_SIZE]);
        binding_id[i] = model.get_object_id(*obj);
    }
}
