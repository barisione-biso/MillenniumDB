
#include "optional_node.h"
#include "left_outer_join.h"

#include <algorithm>
#include <iostream>

#include "base/ids/var_id.h"

using namespace std;

OptionalNode::OptionalNode(std::size_t binding_size,
                                         unique_ptr<BindingIdIter> basic_graph_pattern,
                                         std::vector<std::unique_ptr<BindingIdIter>> children) :
    BindingIdIter(binding_size),
    binding_size(binding_size),
    basic_graph_pattern (move(basic_graph_pattern)),
    children (move(children)) {}


BindingId& OptionalNode::begin(BindingId& input) {

    for (int i = 0; i < children.size(); i++){
        basic_graph_pattern = make_unique<LeftOuterJoin>(binding_size, move(basic_graph_pattern), move(children[i]));
    }
    current_left = &basic_graph_pattern->begin(input);
    return my_binding;
}


void OptionalNode::reset() {
    basic_graph_pattern->reset();
}


bool OptionalNode::next() {
    while (true)
    {
        if (basic_graph_pattern->next())
        {
            my_binding.add_all(*current_left);
            return true;
        }else{
            return false;
        }
    }
}


void OptionalNode::analyze(int indent) const {
    cout << 'OptionalNode(';
}

template class std::unique_ptr<OptionalNode>;
