#ifndef BASE__OP_PROPERTY_H_
#define BASE__OP_PROPERTY_H_

#include <string>

#include "base/ids/node_id.h"
#include "base/parser/logical_plan/op/op.h"

class OpProperty : public Op {
public:
    const NodeId node_id;
    const std::string key;
    const common::ast::Value value;

    std::ostream& print_to_ostream(std::ostream& os, int indent=0) const override {
        os << std::string(indent, ' ');
        os << "OpProperty(" << node_id << "." << key << " = " << value << ")\n";
        return os;
    };

    OpProperty(NodeId node_id, std::string key, common::ast::Value value) :
        node_id (node_id),
        key     (std::move(key)),
        value   (std::move(value)) { }


    void accept_visitor(OpVisitor& visitor) override {
        visitor.visit(*this);
    }


    // Only comparing node_id and key, not checking value
    bool operator<(const OpProperty& other) const {
        if (node_id < other.node_id) {
            return true;
        } else if (node_id > other.node_id) {
            return false;
        } else {
            return key < other.key;
        }
        return false;
    }

    std::set<std::string> get_var_names() const override {
        std::set<std::string> res;
        if (node_id.is_var()) {
            // TODO:
            // res.insert(obj_name);
        }
        // we assume key won't be a variable
        // we assume value won't be a variable
        return res;
    }
};

#endif // BASE__OP_PROPERTY_H_
