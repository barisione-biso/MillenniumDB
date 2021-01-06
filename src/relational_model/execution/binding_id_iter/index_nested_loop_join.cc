#include "index_nested_loop_join.h"

#include <algorithm>
#include <iostream>

#include "base/ids/var_id.h"

using namespace std;

IndexNestedLoopJoin::IndexNestedLoopJoin(std::size_t binding_size,
                                         unique_ptr<BindingIdIter> lhs,
                                         unique_ptr<BindingIdIter> rhs) :
    // BindingIdIter(binding_size),
    lhs (move(lhs)),
    rhs (move(rhs)) { }


void IndexNestedLoopJoin::begin(BindingId& parent_binding, bool parent_has_next) {
    this->parent_binding = &parent_binding;
    if (!parent_has_next) {
        lhs->begin(parent_binding, false);
        rhs->begin(parent_binding, false);
    } else {
        lhs->begin(parent_binding, true);
        if (lhs->next()) {
            rhs->begin(parent_binding, true);
        } else {
            rhs->begin(parent_binding, false);
        }
    }
}

bool IndexNestedLoopJoin::next() {
    while (true) {
        if (rhs->next()) {
            // construct binding
            // my_binding.add_all(*current_left); // TODO: reduntante?
            // my_binding.add_all(*current_right);
            return true;
        } else {
            if (lhs->next())
                rhs->reset();
            else
                return false;
        }
    }
}

void IndexNestedLoopJoin::reset() {
    lhs->reset();
    if (lhs->next())
        rhs->reset();
}


void IndexNestedLoopJoin::assign_nulls() {
    rhs->assign_nulls();
    lhs->assign_nulls();
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

template class std::unique_ptr<IndexNestedLoopJoin>;
