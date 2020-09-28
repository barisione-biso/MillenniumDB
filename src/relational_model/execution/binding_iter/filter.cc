#include "filter.h"

#include <iostream>

#include "base/graph/condition/condition.h"
#include "base/binding/binding.h"
#include "relational_model/execution/binding/binding_filter.h"

using namespace std;

Filter::Filter(GraphModel& model, unique_ptr<BindingIter> iter, unique_ptr<Condition> condition) :
    model     (model),
    iter      (move(iter)),
    condition (move(condition)) { }


void Filter::begin() {
    iter->begin();
}


unique_ptr<Binding> Filter::next() {
    auto next_binding = iter->next();
    while (next_binding != nullptr) {
        auto binding_filter = BindingFilter(model, *next_binding);

        if (condition->eval(binding_filter)) {
            return next_binding;
        } else {
            next_binding = iter->next();
        }
    }
    return nullptr;
}


void Filter::analyze(int indent) const {
    // TODO:
    iter->analyze(indent);
}
