#include "index_nested_loop_join.h"

#include <algorithm>
#include <iostream>

#include "base/ids/var_id.h"

using namespace std;

IndexNestedLoopJoin::IndexNestedLoopJoin(std::size_t binding_size,
                                         unique_ptr<BindingIdIter> lhs,
                                         unique_ptr<BindingIdIter> rhs) :
    BindingIdIter(binding_size),
    lhs (move(lhs)),
    rhs (move(rhs)) { }


BindingId& IndexNestedLoopJoin::begin(BindingId& input) {
    current_left = &lhs->begin(input);
    if (lhs->next()) { // TODO: no se llama begin al rhs (it sera nullptr),
                       // ¿se puede optimizar para no tener que checkear si it sera nullptr?
        current_right = &rhs->begin(*current_left);
    }
    return my_binding;
}


void IndexNestedLoopJoin::reset() {
    lhs->reset();
    if (lhs->next())
        rhs->reset();
}


bool IndexNestedLoopJoin::next() {
    while (true) {
        if (rhs->next()) {
            // construct binding
            my_binding.add_all(*current_left);
            my_binding.add_all(*current_right);
            return true;
        } else {
            if (lhs->next())
                rhs->reset();
            else
                return false;
        }
    }
}


void IndexNestedLoopJoin::analyze(int indent) const {
    // for (int i = 0; i < indent; ++i) {
    //     cout << ' ';
    // }
    // cout << "IndexNestedLoopJoin(\n";
    lhs->analyze(indent);
    cout << ",\n";
    rhs->analyze(indent);
    // cout << "\n";
    // for (int i = 0; i < indent; ++i) {
    //     cout << ' ';
    // }
    // cout << ")";
}
