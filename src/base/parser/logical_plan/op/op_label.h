#ifndef BASE__OP_LABEL_H_
#define BASE__OP_LABEL_H_

#include <string>

#include "base/parser/logical_plan/op/op.h"

class OpLabel : public Op {
public:
    const std::string node_name;
    const std::string label;

    std::ostream& print_to_ostream(std::ostream& os, int indent=0) const override{
        os << std::string(indent, ' ');
        os << "OpLabel(" << node_name << "," << label << ")\n";
        return os;
    };

    OpLabel(std::string node_name, std::string label) :
        node_name (std::move(node_name)),
        label     (std::move(label)    ) { }

    ~OpLabel() = default;


    void accept_visitor(OpVisitor& visitor) const override {
        visitor.visit(*this);
    }

    bool operator<(const OpLabel& other) const {
        if (node_name < other.node_name) {
            return true;
        } else if (label < other.label) {
            return true;
        }
        return false;
    }
};

#endif // BASE__OP_LABEL_H_
