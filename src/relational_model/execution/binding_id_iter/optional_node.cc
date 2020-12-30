
#include "optional_node.h"
#include "left_outer_join.h"

#include <algorithm>
#include <iostream>

#include "base/ids/var_id.h"

using namespace std;

OptionalNode::OptionalNode(std::size_t binding_size,
                           unique_ptr<BindingIdIter> _basic_graph_pattern,
                           std::vector<std::unique_ptr<BindingIdIter>> children) :
    BindingIdIter       (binding_size),
    basic_graph_pattern (move(_basic_graph_pattern)),
    binding_size        (binding_size)
{
    // TEST 1
    for (auto& child : children) {
        basic_graph_pattern = make_unique<LeftOuterJoin>(binding_size, move(basic_graph_pattern), move(child));
    }
    children.clear(); // TODO: TEST
}


BindingId& OptionalNode::begin(BindingId& input) {
    current_left = &basic_graph_pattern->begin(input);
    return my_binding;
}


void OptionalNode::reset() {
    basic_graph_pattern->reset();
}


bool OptionalNode::next() {
    while (true) {
        if (basic_graph_pattern->next()) {
            my_binding.add_all(*current_left);
            return true;
        } else {
            return false;
        }
    }
}


void OptionalNode::analyze(int indent) const {
    // TODO CRIS
    cout << "OptionalNode(\n";
    basic_graph_pattern->analyze(indent);
    cout << ")\n";
}

template class std::unique_ptr<OptionalNode>;
