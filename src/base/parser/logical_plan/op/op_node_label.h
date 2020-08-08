#ifndef BASE__OP_NODE_LABEL_H_
#define BASE__OP_NODE_LABEL_H_

#include <string>

#include "base/ids/graph_id.h"
#include "base/graph/graph_object.h"
#include "base/parser/logical_plan/op/op.h"

class OpNodeLabel : public Op {
public:
    const std::string graph_name;
    const std::string node_name;
    const std::string label;

    OpNodeLabel(std::string graph_name, std::string node_name, std::string label) :
        graph_name(std::move(graph_name)),
        node_name (std::move(node_name) ),
        label     (std::move(label)     ) { }

    ~OpNodeLabel() = default;

    void accept_visitor(OpVisitor& visitor) {
        visitor.visit(*this);
    }

    bool operator<(const OpNodeLabel& other) const {
        if (graph_name < other.graph_name) {
            return true;
        } else if (node_name < other.node_name) {
            return true;
        } else if (label < other.label) {
            return true;
        }
        return false;
    }
};

#endif // BASE__OP_NODE_LABEL_H_
