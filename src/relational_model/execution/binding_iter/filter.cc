#include "filter.h"

#include "base/graph/condition/condition.h"
#include "base/binding/binding.h"
#include "relational_model/binding/binding_filter.h"

#include <iostream>

using namespace std;

Filter::Filter(unique_ptr<BindingIter> iter, unique_ptr<Condition> condition, map<string, GraphId> var2graph_id,
               set<string> node_names, set<string> edge_names) :
    iter(move(iter)),
    condition(move(condition)),
    var2graph_id(move(var2graph_id)),
    node_names(move(node_names)),
    edge_names(move(edge_names)) { }


void Filter::begin() {
    iter->begin();
}


unique_ptr<Binding> Filter::next() {
    auto next_binding = iter->next();
    while (next_binding != nullptr) {
        auto binding_filter = BindingFilter(*next_binding, var2graph_id, node_names, edge_names);

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
