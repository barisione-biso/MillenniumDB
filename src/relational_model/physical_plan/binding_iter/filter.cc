#include "filter.h"

#include "base/graph/condition/condition.h"
#include "base/binding/binding.h"

#include <iostream>

using namespace std;

Filter::Filter(unique_ptr<BindingIter> iter, unique_ptr<Condition> condition, map<string, VarId> id_map)
    : iter(move(iter)), condition(move(condition)), id_map(move(id_map)) { }


void Filter::begin() {
    iter->begin();
    // TODO: construct list of bindings needed to eval the filter that are not present in the binding
}


std::unique_ptr<Binding> Filter::next() {
    auto next = iter->next();
    while (next != nullptr) {
        if (condition->eval(*next)) {
            return next;
        }
        next = iter->next();
    }
    return nullptr;
}
