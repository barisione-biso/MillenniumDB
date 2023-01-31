#pragma once

#include <memory>
#include <vector>

#include "parser/query/op/op.h"

namespace SPARQL {
class OpUnion : public Op {
public:
    std::vector<std::unique_ptr<Op>> unions;

    OpUnion(std::vector<std::unique_ptr<Op>>&& unions) : unions(std::move(unions)) { }

    void accept_visitor(OpVisitor& visitor) override {
        visitor.visit(*this);
    }

    std::set<Var> get_vars() const override {
        std::set<Var> res;
        for (auto& child : unions) {
            for (auto& child_var : child->get_vars()) {
                res.insert(child_var);
            }
        }
        return res;
    }

    std::ostream& print_to_ostream(std::ostream& os, int indent = 0) const override {
        os << std::string(indent, ' ');
        os << "OpUnion(\n";
        for (auto& child : unions) {
            child->print_to_ostream(os, indent + 2);
        }
        os << std::string(indent, ' ');
        os << ")\n";
        return os;
    }
};
} // namespace SPARQL