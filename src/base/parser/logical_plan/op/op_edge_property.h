#ifndef BASE__OP_EDGE_PROPERTY_H_
#define BASE__OP_EDGE_PROPERTY_H_

#include <string>

#include "base/ids/graph_id.h"
#include "base/graph/graph_object.h"
#include "base/parser/logical_plan/op/op.h"

class OpEdgeProperty : public Op {
public:
    const std::string graph_name;
    const std::string edge_name;
    const std::string key;
    const common::ast::Value value;

    OpEdgeProperty(std::string graph_name, std::string edge_name, std::string key, common::ast::Value value) :
        graph_name(std::move(graph_name)),
        edge_name (std::move(edge_name) ),
        key       (std::move(key)       ),
        value     (std::move(value)     ) { }

    void accept_visitor(OpVisitor& visitor) {
        visitor.visit(*this);
    }

    bool operator<(const OpEdgeProperty& other) const {
        if (graph_name < other.graph_name) {
            return true;
        } else if (edge_name < other.edge_name) {
            return true;
        } else if (key < other.key) {
            return true;
        }
        // not checking value
        return false;
    }
};

#endif // BASE__OP_EDGE_PROPERTY_H_
