#ifndef BASE__OP_EDGE_LABEL_H_
#define BASE__OP_EDGE_LABEL_H_

#include <string>

#include "base/ids/graph_id.h"
#include "base/graph/graph_object.h"
#include "base/parser/logical_plan/op/op.h"

class OpEdgeLabel : public Op {
public:
    std::string graph_name;
    std::string edge_name;
    std::string label;

    OpEdgeLabel(std::string graph_name, std::string edge_name, std::string label) :
        graph_name(std::move(graph_name)),
        edge_name (std::move(edge_name) ),
        label     (std::move(label)     ) { }

    ~OpEdgeLabel() = default;

    void accept_visitor(OpVisitor& visitor) {
        visitor.visit(*this);
    }

    bool operator<(const OpEdgeLabel& other) const {
        if (graph_name < other.graph_name) {
            return true;
        } else if (edge_name < other.edge_name) {
            return true;
        } else if (label < other.label) {
            return true;
        }
        return false;
    }

    OpEdgeLabel& operator=(const OpEdgeLabel& other) {
        graph_name = other.graph_name;
        edge_name  = other.edge_name;
        label      = other.label;
        return *this;
    }
};

#endif // BASE__OP_EDGE_LABEL_H_
