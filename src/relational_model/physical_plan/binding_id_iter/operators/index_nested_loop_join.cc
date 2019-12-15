#include "index_nested_loop_join.h"

#include <algorithm>

#include "base/var/var_id.h"
#include "relational_model/config.h"
#include "relational_model/physical_plan/binding_id.h"

using namespace std;

IndexNestedLoopJoin::IndexNestedLoopJoin(Config& config, BindingIdIter& left, BindingIdIter& right)
    : config(config), left(left), right(right)
{
}

void IndexNestedLoopJoin::init(shared_ptr<BindingId> input) {
    left.init(input);
    current_left = left.next();
    right.init(current_left);
}


unique_ptr<BindingId> IndexNestedLoopJoin::next() {
    while (current_left != nullptr) {
        current_right = right.next();

        if (current_right != nullptr) {
            return construct_binding(*current_left, *current_right);
        }
        else {
            current_left = left.next();
            right.reset(current_left);
        }
    }
    return nullptr;
}


unique_ptr<BindingId> IndexNestedLoopJoin::construct_binding(BindingId& lhs, BindingId& rhs) {
    auto result = make_unique<BindingId>(lhs.var_count());
    result->add_all(lhs);
    result->add_all(rhs);
    return result;
}


void IndexNestedLoopJoin::reset(shared_ptr<BindingId> input) {
    left.reset(input);
    current_left = left.next();
    right.reset(current_left);
}
