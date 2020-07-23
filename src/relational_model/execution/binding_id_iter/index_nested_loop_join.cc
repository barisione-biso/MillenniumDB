#include "index_nested_loop_join.h"

#include <algorithm>
#include <iostream>

#include "base/ids/var_id.h"

using namespace std;

IndexNestedLoopJoin::IndexNestedLoopJoin(unique_ptr<BindingIdIter> lhs, unique_ptr<BindingIdIter> rhs) :
    lhs(move(lhs)), rhs(move(rhs)) { }


void IndexNestedLoopJoin::begin(BindingId& input) {
    my_binding = make_unique<BindingId>(input.var_count());
    lhs->begin(input);
    current_left = lhs->next();
    if (current_left != nullptr)
        rhs->begin(*current_left);
}


void IndexNestedLoopJoin::reset(BindingId& input) {
    lhs->reset(input);
    if (current_left != nullptr)
        rhs->reset(*current_left);
}


BindingId* IndexNestedLoopJoin::next() {
    while (current_left != nullptr) {
        current_right = rhs->next();

        if (current_right != nullptr) {
            construct_binding();
            return my_binding.get();
        }
        else {
            current_left = lhs->next();
            if (current_left != nullptr)
                rhs->reset(*current_left);
        }
    }
    return nullptr;
}


void IndexNestedLoopJoin::construct_binding() {
    my_binding->add_all(*current_left);
    my_binding->add_all(*current_right);
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
