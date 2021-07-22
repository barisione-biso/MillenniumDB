#include "index_left_outer_join.h"

#include <algorithm>
#include <iostream>

#include "base/ids/var_id.h"
#include "relational_model/execution/binding_id_iter/empty_binding_id_iter.h"

using namespace std;

IndexLeftOuterJoin::IndexLeftOuterJoin(unique_ptr<BindingIdIter> _lhs,
                                       unique_ptr<BindingIdIter> _rhs) :
    lhs          (move(_lhs)),
    original_rhs (move(_rhs)) { }


void IndexLeftOuterJoin::begin(BindingId& parent_binding) {
    this->parent_binding = &parent_binding;

    lhs->begin(parent_binding);
    if (lhs->next()) {
        must_return_null = true;
        rhs = original_rhs.get();
    } else {
        must_return_null = false;
        rhs = &EmptyBindingIdIter::instance;
    }
    original_rhs->begin(parent_binding);
}


bool IndexLeftOuterJoin::next() {
    while (true) {
        if (rhs->next()) {
            must_return_null = false;
            ++results_found;
            return true;
        } else {
            if (must_return_null) {
                rhs->assign_nulls();
                must_return_null = false;
                ++results_found;
                return true;
            } else {
                if (lhs->next()) {
                    must_return_null = true;
                    rhs->reset();
                } else {
                    must_return_null = false;
                    return false;
                }
            }
        }
    }
}


void IndexLeftOuterJoin::reset() {
    lhs->reset();
    if (lhs->next()) {
        must_return_null = true;
        rhs = original_rhs.get();
        rhs->reset();
    } else {
        must_return_null = false;
        rhs = &EmptyBindingIdIter::instance;
    }
}


void IndexLeftOuterJoin::assign_nulls() {
    lhs->assign_nulls();
    original_rhs->assign_nulls();
}


void IndexLeftOuterJoin::analyze(int indent) const {
    for (int i = 0; i < indent; ++i) {
        cout << ' ';
    }
    cout << "IndexLeftOuterJoin(\n";
    lhs->analyze(indent + 2);
    cout << ",\n";
    original_rhs->analyze(indent + 2);
    cout << "\n";
    for (int i = 0; i < indent; ++i) {
        cout << ' ';
    }
    cout << ") Results found: " << results_found;
}

template class std::unique_ptr<IndexLeftOuterJoin>;
