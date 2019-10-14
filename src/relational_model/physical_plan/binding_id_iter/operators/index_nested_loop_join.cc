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

void IndexNestedLoopJoin::init(shared_ptr<BindingId const> input)
{
    left.init(input);
    current_left = left.next();
    right.init(current_left);
}

unique_ptr<BindingId const> IndexNestedLoopJoin::next()
{
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


unique_ptr<BindingId const> IndexNestedLoopJoin::construct_binding(BindingId const& lhs, BindingId const& rhs)
{
    auto result = make_unique<BindingId>();
    result->add(lhs.get_values());
    result->try_add(rhs.get_values());
    return result;
}

void IndexNestedLoopJoin::reset(shared_ptr<BindingId const> input)
{
    left.reset(input);
    current_left = left.next();
    right.reset(current_left);
}
