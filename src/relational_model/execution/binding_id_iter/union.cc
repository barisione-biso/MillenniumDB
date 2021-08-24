#include "union.h"

#include <iostream>

using namespace std;

Union::Union(std::vector<std::unique_ptr<BindingIdIter>> iters) :
    iters(move(iters)) { }


void Union::begin(BindingId& parent_binding) {
    current_iter = 0;
    for (auto& iter : iters) {
        iter->begin(parent_binding);
    }
}


bool Union::next() {
    while (current_iter < iters.size()) {
        if (iters[current_iter]->next()) {
            return true;
        } else {
            ++current_iter;
        }
    }
    return false;
}


void Union::reset() {
    current_iter = 0;
    for (auto& iter : iters) {
        iter->reset();
    }
}


void Union::assign_nulls() {
    iters[current_iter]->assign_nulls();
}


void Union::analyze(int indent) const {
    cout << "Union:\n";
    for (const auto& iter : iters) {
        iter->analyze(indent + 2);
        cout << "\n";
    }
}
