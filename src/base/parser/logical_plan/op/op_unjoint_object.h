#ifndef BASE__OP_UNJOINT_OBJECT_H_
#define BASE__OP_UNJOINT_OBJECT_H_

#include <string>

#include "base/ids/node_id.h"
#include "base/parser/logical_plan/op/op.h"

class OpUnjointObject : public Op {
public:
    const NodeId node_id;

    std::ostream& print_to_ostream(std::ostream& os, int indent=0) const override {
        os << std::string(indent, ' ');
        os << "OpUnjointObject(" << node_id << ")\n";
        return os;
    };

    OpUnjointObject(NodeId node_id) :
        node_id (node_id) { }

    ~OpUnjointObject() = default;


    void accept_visitor(OpVisitor& visitor) override {
        visitor.visit(*this);
    }

    bool operator<(const OpUnjointObject& other) const {
        return node_id < other.node_id;
    }

    std::set<std::string> get_var_names() const override {
        std::set<std::string> res;
        if (node_id.is_var()) {
            // TODO:
        }
        return res;
    }
};

#endif // BASE__OP_UNJOINT_OBJECT_H_
