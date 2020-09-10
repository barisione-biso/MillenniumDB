#ifndef BASE__OP_NODE_PROPERTY_H_
#define BASE__OP_NODE_PROPERTY_H_

#include <string>

#include "base/ids/graph_id.h"
#include "base/graph/graph_object.h"
#include "base/parser/logical_plan/op/op.h"

class OpNodeProperty : public Op {
public:
    const std::string graph_name;
    const std::string node_name;
    const std::string key;
    const common::ast::Value value;

    OpNodeProperty(std::string graph_name, std::string node_name, std::string key, common::ast::Value value) :
        graph_name(std::move(graph_name)),
        node_name (std::move(node_name) ),
        key       (std::move(key)       ),
        value     (std::move(value)     ) { }

    void accept_visitor(OpVisitor& visitor) {
        visitor.visit(*this);
    }

    bool operator<(const OpNodeProperty& other) const {
        if (graph_name < other.graph_name) {
            return true;
        } else if (node_name < other.node_name) {
            return true;
        } else if (key < other.key) {
            return true;
        }
        // not checking value
        return false;
    }
};

#endif // BASE__OP_NODE_PROPERTY_H_
