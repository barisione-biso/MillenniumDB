#include "binding_order_by.h"

#include <cassert>
#include <iostream>

#include "base/binding/binding.h"
#include "base/graph/value/value.h"

using namespace std;


BindingOrderBy::BindingOrderBy(GraphModel& model, vector<pair<string, VarId>> order_vars, Binding& child_binding, size_t binding_size) :
    model         (model),
    order_vars    (move(order_vars)),
    child_binding (child_binding),
    binding_size  (binding_size)
{
    binding_id = new ObjectId(binding_size);
}


BindingOrderBy::~BindingOrderBy() {
    delete binding_id;
}


std::string BindingOrderBy::to_string() const {
    // std::string result;
    // result += '{';
    // auto it = order_vars.cbegin();

    // while (true) {
    //     auto& var_varid_pair = *it;
    //     result += var_varid_pair.first;
    //     result += ':';
    //     result += child_binding[var_varid_pair.second]->to_string();
    //     ++it;
    //     if (it != order_vars.cend()) {
    //         result += ',';
    //     } else {
    //         result +=  "}\n";
    //         return result;
    //     }
    // }
}


shared_ptr<GraphObject> BindingOrderBy::operator[](const VarId var) {
    assert(var.id < binding_size);
    return model.get_graph_object(binding_id[var.id]);
}


void BindingOrderBy::update_binding_object(std::vector<uint64_t> obj_ids) {
    assert(obj_ids.size() == binding_size);
    for (size_t i = 0; i < binding_size; i++) {
        binding_id[i] = ObjectId(obj_ids[i]);
    }
}


ObjectId BindingOrderBy::get_id(const VarId var_id) {
    return child_binding.get_id(var_id);
}
