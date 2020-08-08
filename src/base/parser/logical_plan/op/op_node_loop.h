#ifndef BASE__OP_NODE_LOOP_H_
#define BASE__OP_NODE_LOOP_H_

#include "base/ids/graph_id.h"
#include "base/parser/logical_plan/op/op.h"

#include <string>

class OpNodeLoop : public Op {
public:
    const std::string graph_name;
    const std::string node;
    const std::string edge;

    OpNodeLoop(std::string graph_name, std::string node, std::string edge) :
        graph_name(std::move(graph_name)),
        node      (std::move(node)      ),
        edge      (std::move(edge)      ) { }

    void accept_visitor(OpVisitor& visitor) {
        visitor.visit(*this);
    }

    bool operator<(const OpNodeLoop& other) const {
        if (graph_name < other.graph_name) {
            return true;
        } else if (node < other.node) {
            return true;
        } else if (edge < other.edge) {
            return true;
        }
        return false;
    }
};

#endif // BASE__OP_NODE_LOOP_H_
