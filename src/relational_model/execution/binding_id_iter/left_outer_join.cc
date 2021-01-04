
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
    rhs           (move(rhs)),
    current_left  (binding_size),
    current_right (binding_size) { }


void LeftOuterJoin::begin(BindingId& parent_binding, bool parent_has_next) {
    has_result = false;
    this->parent_binding = &parent_binding;
    current_left.add_all(parent_binding);
    current_right.add_all(parent_binding);
    if (!parent_has_next) {
        has_left = false;
        lhs->begin(current_left, false);
        rhs->begin(current_right, false);
    } else {
        lhs->begin(current_left, true);
        if (lhs->next()) {
            current_right.add_all(current_left); // NEW
            has_left = true;
            rhs->begin(current_right, true);
        } else {
            has_left = false;
            rhs->begin(current_right, false);
        }
    }
}


void LeftOuterJoin::reset() {
    has_result = false;
    has_left = true;
    lhs->reset();
    if (lhs->next()) {
        current_right.add_all(current_left); // NEW
        rhs->reset();
    } else {
        has_left = false;
    }
}


bool LeftOuterJoin::next() {
    if(!has_left) {
        return false;
    }
    while (true) {
        if (rhs->next()) {
            has_result = true;
            parent_binding->add_all(current_left);
            parent_binding->add_all(current_right);
            return true;
        } else {
            if (!has_result) {
                parent_binding->add_all(current_left);
                has_result = true;
                return true;
            } else {
                if (lhs->next()) {
                    current_right.add_all(current_left); // NEW
                    has_result = false;
                    rhs->reset();
                } else {
                    return false;
                }
            }
        }
    }
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
