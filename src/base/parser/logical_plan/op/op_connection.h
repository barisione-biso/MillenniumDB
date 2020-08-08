#ifndef BASE__OP_CONNECTION_H_
#define BASE__OP_CONNECTION_H_

#include <string>

#include "base/ids/graph_id.h"
#include "base/parser/logical_plan/op/op.h"

class OpConnection : public Op {
public:
    const std::string graph_name;
    const std::string node_from;
    const std::string node_to;
    const std::string edge;

    OpConnection(std::string graph_name, std::string node_from, std::string edge, std::string node_to) :
        graph_name(std::move(graph_name)),
        node_from (std::move(node_from) ),
        node_to   (std::move(node_to)   ),
        edge      (std::move(edge)      ) { }

    void accept_visitor(OpVisitor& visitor) {
        visitor.visit(*this);
    }

    bool operator<(const OpConnection& other) const {
        if (graph_name < other.graph_name) {
            return true;
        } else if (node_from < other.node_from) {
            return true;
        } else if (node_to < other.node_to) {
            return true;
        } else if (edge < other.edge) {
            return true;
        }
        return false;
    }
};

#endif // BASE__OP_CONNECTION_H_
