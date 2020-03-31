#include "filter.h"

#include "base/graph/condition/condition.h"
#include "base/binding/binding.h"
#include "relational_model/binding/binding_filter.h"

#include <iostream>

using namespace std;

Filter::Filter(unique_ptr<BindingIter> iter, unique_ptr<Condition> condition,
               map<string, GraphId> graph_ids, map<string, ObjectType> element_types)
    : iter(move(iter)), condition(move(condition)), graph_ids(move(graph_ids)), element_types(move(element_types)) { }


void Filter::begin() {
    iter->begin();
}


std::unique_ptr<Binding> Filter::next() {
    auto next_binding = iter->next();
    while (next_binding != nullptr) {
        auto binding_filter = BindingFilter(*next_binding, graph_ids, element_types);

        if (condition->eval(binding_filter)) {
            return next_binding;
        }
        else {
            next_binding = iter->next();
        }
    }
    return nullptr;
}
