#include "where.h"

#include <iostream>

#include "base/binding/binding.h"
#include "base/graph/condition/condition.h"

using namespace std;

Where::Where(const GraphModel& model,
             unique_ptr<BindingIter> _child_iter,
             unique_ptr<Condition> condition,
             std::size_t child_binding_size,
             std::map<VarId, std::pair<VarId, ObjectId>> property_map) :
    model      (model),
    child_iter (move(_child_iter)),
    condition  (move(condition)),
    my_binding (BindingWhere(model, child_iter->get_binding(), VarId(child_binding_size-1), move(property_map))) { }


void Where::begin() {
    child_iter->begin();
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
