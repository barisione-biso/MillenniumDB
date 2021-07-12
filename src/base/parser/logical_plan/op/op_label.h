#ifndef BASE__OP_LABEL_H_
#define BASE__OP_LABEL_H_

#include <string>

#include "base/ids/node_id.h"
#include "base/parser/logical_plan/op/op.h"

class OpLabel : public Op {
public:
    const NodeId node_id; // TODO: solo deberia poder ser un node_name o var
    const std::string label;

    std::ostream& print_to_ostream(std::ostream& os, int indent=0) const override{
        os << std::string(indent, ' ');
        os << "OpLabel(" << node_id << "," << label << ")\n";
        return os;
    };

    OpLabel(NodeId node_id, std::string label) :
        node_id (std::move(node_id)),
        label   (std::move(label)) { }

    ~OpLabel() = default;


    void accept_visitor(OpVisitor& visitor) override {
        visitor.visit(*this);
    }

    bool operator<(const OpLabel& other) const {
        if (node_id < other.node_id) {
            return true;
        } else if (node_id > other.node_id) {
            return false;
        } else {
            return label < other.label;
        }
    }

    void get_vars(std::set<Var>& set) const override {
        if (node_id.is_var()) {
            set.insert(node_id.to_var());
        }
        // we assume label won't be a variable
    }
};

#endif // BASE__OP_LABEL_H_
