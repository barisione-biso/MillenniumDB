#include "merge_join.h"

#include <cassert>

using namespace std;

MergeJoin::MergeJoin(unique_ptr<BindingIdIter> lhs, unique_ptr<BindingIdIter> rhs,
                     vector<VarId> join_vars) :
    lhs(move(lhs)), rhs(move(rhs)), join_vars(move(join_vars))
{
    assert(join_vars.size() > 0
        && "Join vars cannot be empty");
}


void MergeJoin::begin(BindingId& input) {
    my_binding = make_unique<BindingId>(input.var_count());

    lhs->begin(input);
    current_left = lhs->next();

    rhs->begin(input);
    current_right = rhs->next();
}


void MergeJoin::reset(BindingId& input) {
    lhs->reset(input);
    current_left = lhs->next();

    rhs->reset(input);
    current_right = rhs->next();
}


BindingId* MergeJoin::next() {
    while (current_left != nullptr && current_right != nullptr) {
        if (left_compatible_with_right()) {
            construct_binding();
            return my_binding.get();
        } else {
            if ( left_less_than_right() ) {
                current_left = lhs->next();
            } else {
                current_right = rhs->next();
            }
        }
    }
    return nullptr;
}

bool MergeJoin::left_compatible_with_right() {
    for (auto& var : join_vars) {
        if ((*current_left)[var] != (*current_left)[var]) {
            return false;
        }
    }
    return true;
}


bool MergeJoin::left_less_than_right() {
    for (auto& var : join_vars) {
        if ((*current_left)[var] < (*current_left)[var]) {
            return true;
        }
    }
    return false;
}


void MergeJoin::construct_binding() {
    my_binding->add_all(*current_left);
    my_binding->add_all(*current_right);
}