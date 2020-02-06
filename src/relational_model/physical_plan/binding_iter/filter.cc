#include "filter.h"

#include "base/graph/condition/condition.h"
#include "base/binding/binding.h"

#include <iostream>

using namespace std;

Filter::Filter(std::unique_ptr<BindingIter> iter, std::unique_ptr<Condition> condition)
    : iter(move(iter)), condition(move(condition)) {
    // if (this->iter == nullptr) {
    //     cout << "Filter: iter null\n";
    // }
    // else {
    //     cout << "Filter: iter not null\n";
    // }
}

void Filter::begin() {
    cout << "Filter begin\n";

    iter->begin();
}


std::unique_ptr<Binding> Filter::next() {
    auto next = iter->next();
    while (next != nullptr) {
        if (condition->eval(*next)) {
            return next;
        }
        else {
            cout << "Filtered result\n";
        }
        next = iter->next();
    }
    return nullptr;
}
