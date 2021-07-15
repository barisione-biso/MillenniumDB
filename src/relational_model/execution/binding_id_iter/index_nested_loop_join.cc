#include "index_nested_loop_join.h"

#include <algorithm>
#include <iostream>

#include "base/ids/var_id.h"
#include "relational_model/execution/binding_id_iter/empty_binding_id_iter.h"

using namespace std;

IndexNestedLoopJoin::IndexNestedLoopJoin(unique_ptr<BindingIdIter> _lhs,
                                         unique_ptr<BindingIdIter> _rhs) :
    lhs          (move(_lhs)),
    original_rhs (move(_rhs)) { }


void IndexNestedLoopJoin::begin(BindingId& parent_binding) {
    this->parent_binding = &parent_binding;

    lhs->begin(parent_binding);
    if (lhs->next()) {
        rhs = original_rhs.get();
        rhs->begin(parent_binding);
    } else {
        rhs = &EmptyBindingIdIter::instance;
    }
}

bool IndexNestedLoopJoin::next() {
    while (true) {
        if (rhs->next()) {
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
    for (int i = 0; i < indent; ++i) {
         cout << ' ';
    }
    cout << "IndexNestedLoopJoin(\n";
    lhs->analyze(indent + 2);
    cout << ",\n";
    rhs->analyze(indent + 2);
    cout << "\n";
    for (int i = 0; i < indent; ++i) {
        cout << ' ';
    }
    cout << ")";
}

template class std::unique_ptr<IndexNestedLoopJoin>;
