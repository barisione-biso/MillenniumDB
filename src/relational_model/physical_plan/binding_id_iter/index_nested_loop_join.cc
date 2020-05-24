#include "index_nested_loop_join.h"

#include <algorithm>

#include "base/ids/var_id.h"

using namespace std;

IndexNestedLoopJoin::IndexNestedLoopJoin(unique_ptr<BindingIdIter> left, unique_ptr<BindingIdIter> right) :
    left(move(left)), right(move(right)) { }


void IndexNestedLoopJoin::begin(BindingId& input) {
    my_binding = make_unique<BindingId>(input.var_count());
    left->begin(input);
    current_left = left->next();
    if (current_left != nullptr)
        right->begin(*current_left);
}


void IndexNestedLoopJoin::reset(BindingId& input) {
    left->reset(input);
    if (current_left != nullptr)
        right->reset(*current_left);
}


BindingId* IndexNestedLoopJoin::next() {
    while (current_left != nullptr) {
        current_right = right->next();

        if (current_right != nullptr) {
            construct_binding(*current_left, *current_right);
            return my_binding.get();
        }
        else {
            current_left = left->next();
            if (current_left != nullptr)
                right->reset(*current_left);
        }
    }
    return nullptr;
}

// TODO: solo se recibe *current_left, *current_right, se puede eliminar
void IndexNestedLoopJoin::construct_binding(BindingId& lhs, BindingId& rhs) {
    my_binding->add_all(lhs);
    my_binding->add_all(rhs);
}
