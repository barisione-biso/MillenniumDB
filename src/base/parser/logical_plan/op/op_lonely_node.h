#ifndef BASE__OP_LONELY_NODE_H_
#define BASE__OP_LONELY_NODE_H_

#include "base/ids/graph_id.h"
#include "base/parser/logical_plan/op/op.h"

#include <string>

class OpLonelyNode : public Op {
public:
    const std::string graph_name;
    const std::string node_name;

    OpLonelyNode(std::string graph_name, std::string node_name):
        graph_name(std::move(graph_name)),
        node_name (std::move(node_name )) { }

    ~OpLonelyNode() = default;

    void accept_visitor(OpVisitor& visitor) {
        visitor.visit(*this);
    }

    bool operator<(const OpLonelyNode& other) const {
        if (graph_name < other.graph_name) {
            return true;
        } else if (node_name < other.node_name) {
            return true;
        }
        return false;
    }
};

#endif // BASE__OP_LONELY_NODE_H_
