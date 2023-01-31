#pragma once

#include <memory>
#include <vector>

#include "parser/query/op/op.h"

namespace SPARQL {

class OpMinus : public Op {
public:
    std::unique_ptr<Op> lhs;
    std::unique_ptr<Op> rhs;

    OpMinus(std::unique_ptr<Op> lhs, std::unique_ptr<Op> rhs) :
        lhs(std::move(lhs)), rhs(std::move(rhs)) { }

    void accept_visitor(OpVisitor& visitor) override {
        visitor.visit(*this);
    }

    std::set<Var> get_vars() const override {
        auto res = lhs->get_vars();
        auto rhs_vars = rhs->get_vars();
        res.insert(rhs_vars.begin(), rhs_vars.end());
        return res;
    }

    std::ostream& print_to_ostream(std::ostream& os, int indent = 0) const override {
        os << std::string(indent, ' ');
        os << "OpMinus(\n";
        lhs->print_to_ostream(os, indent + 2);
        rhs->print_to_ostream(os, indent + 2);
        os << std::string(indent, ' ');
        os << ")\n";
        return os;
    }
};
} // namespace SPARQL