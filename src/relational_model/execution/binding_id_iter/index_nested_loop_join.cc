#include "index_nested_loop_join.h"

#include <algorithm>
#include <iostream>

#include "base/ids/var_id.h"

using namespace std;

IndexNestedLoopJoin::IndexNestedLoopJoin(unique_ptr<BindingIdIter> lhs, unique_ptr<BindingIdIter> rhs) :
    lhs (move(lhs)),
    rhs (move(rhs)) { }


BindingId& IndexNestedLoopJoin::begin(BindingId& input) {
    my_binding.init(input.var_count());
    current_left = &lhs->begin(input);
    if (lhs->next()) {
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
            construct_binding();
            return true;
        } else {
            if (lhs->next())
                rhs->reset();
            else
                return false;
        }
    }
}


void IndexNestedLoopJoin::construct_binding() {
    my_binding.add_all(*current_left);
    my_binding.add_all(*current_right);
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
