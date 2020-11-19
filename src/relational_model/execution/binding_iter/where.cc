#include "where.h"

#include <iostream>

#include "base/graph/condition/condition.h"
#include "base/binding/binding.h"

using namespace std;

Where::Where(GraphModel& model, unique_ptr<BindingIter> _child_iter, unique_ptr<Condition> _condition,
             std::size_t child_binding_size, std::map<VarId, std::pair<VarId, ObjectId>> property_map) :
    model      (model),
    child_iter (move(_child_iter)),
    condition  (move(_condition)),
    my_binding (BindingWhere(model, child_iter->get_binding(), VarId(child_binding_size-1), move(property_map))) { }


Binding& Where::get_binding() {
    return my_binding;
}


bool Where::next() {
    while (child_iter->next()) {
        my_binding.clear_cache();
        if (condition->eval(my_binding)) {
            ++results;
            return true;
        }
    }
    return false;
}


void Where::analyze(int indent) const {
    std::cout << "Where(found: " << results << ")\n";
    child_iter->analyze(indent+2);
}
