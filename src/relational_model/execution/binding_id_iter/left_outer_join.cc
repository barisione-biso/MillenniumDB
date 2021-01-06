
#include "left_outer_join.h"

#include <algorithm>
#include <iostream>

#include "base/ids/var_id.h"

using namespace std;

LeftOuterJoin::LeftOuterJoin(std::size_t binding_size,
                             unique_ptr<BindingIdIter> lhs,
                             unique_ptr<BindingIdIter> rhs) :
    // BindingIdIter(binding_size),
    lhs           (move(lhs)),
    rhs           (move(rhs)) { }


void LeftOuterJoin::begin(BindingId& parent_binding, bool parent_has_next) {
    has_result = false;
    this->parent_binding = &parent_binding;
    if (!parent_has_next) {
        has_left = false;
        lhs->begin(parent_binding, false);
        rhs->begin(parent_binding, false);
    } else {
        lhs->begin(parent_binding, true);
        if (lhs->next()) {
            has_left = true;
            rhs->begin(parent_binding, true);
        } else {
            has_left = false;
            rhs->begin(parent_binding, false);
        }
    }
}


bool LeftOuterJoin::next() {
    if (!has_left) {
        return false;
    }
    while (true) {
        if (rhs->next()) {
            has_result = true;
            return true;
        } else {
            if (!has_result) {
                rhs->assign_nulls();
                has_result = true;
                return true;
            } else {
                if (lhs->next()) {
                    has_result = false;
                    rhs->reset();
                } else {
                    return false;
                }
            }
        }
    }
}


void LeftOuterJoin::reset() {
    has_result = false;
    lhs->reset();
    if (lhs->next()) {
        rhs->reset();
        has_left = true;
    } else {
        has_left = false;
    }
}


void LeftOuterJoin::assign_nulls() {
    lhs->assign_nulls();
    rhs->assign_nulls();
}


void LeftOuterJoin::analyze(int indent) const {
    // for (int i = 0; i < indent; ++i) {
    //     cout << ' ';
    // }
    // cout << "LeftOuterJoin(\n";
    lhs->analyze(indent);
    cout << ",\n";
    rhs->analyze(indent);
    // cout << "\n";
    // for (int i = 0; i < indent; ++i) {
    //     cout << ' ';
    // }
    // cout << ")";
}

template class std::unique_ptr<LeftOuterJoin>;
