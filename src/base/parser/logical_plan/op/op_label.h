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


    void accept_visitor(OpVisitor& visitor) override {
        visitor.visit(*this);
    }

    bool operator<(const OpLabel& other) const {
        if (node_name < other.node_name) {
            return true;
        } else if (node_name > other.node_name) {
            return false;
        } else {
            return label < other.label;
        }
    }

    std::set<std::string> get_var_names() const override {
        std::set<std::string> res;
        if (node_name[0] == '?') {
            res.insert(node_name);
        }
        if (label[0] == '?') {
            res.insert(label);
        }
        return res;
    }
};

#endif // BASE__OP_LABEL_H_
