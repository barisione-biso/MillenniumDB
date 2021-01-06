
#include "optional_node.h"

#include <algorithm>
#include <iostream>

#include "base/ids/var_id.h"
#include "left_outer_join.h"

using namespace std;

OptionalNode::OptionalNode(std::size_t binding_size,
                           unique_ptr<BindingIdIter> _graph_pattern,
                           std::vector<std::unique_ptr<BindingIdIter>> children) :
    graph_pattern (move(_graph_pattern)),
    binding_size  (binding_size)
{
    for (auto& child : children) {
        graph_pattern = make_unique<LeftOuterJoin>(binding_size, move(graph_pattern), move(child));
    }
}


void OptionalNode::begin(BindingId& parent_binding, bool parent_has_next) {
    graph_pattern->begin(parent_binding, parent_has_next);
}


bool OptionalNode::next() {
    return graph_pattern->next();
}


void OptionalNode::reset() {
    graph_pattern->reset();
}


void OptionalNode::assign_nulls() {
    graph_pattern->assign_nulls();
}


void OptionalNode::analyze(int indent) const {
    cout << "OptionalNode(\n";
    graph_pattern->analyze(indent);
    cout << ")\n";
}

template class std::unique_ptr<OptionalNode>;
