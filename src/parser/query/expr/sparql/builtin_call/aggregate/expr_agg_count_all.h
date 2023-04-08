#pragma once

#include <memory>

#include "parser/query/expr/expr.h"

namespace SPARQL {
class ExprAggCountAll : public Expr {
public:
    bool distinct;

    ExprAggCountAll(bool distinct) :
        distinct (distinct) { }

    void accept_visitor(ExprVisitor& visitor) override {
        visitor.visit(*this);
    }

    virtual std::set<Var> get_vars() const override {
        return std::set<Var>();
    }

    virtual std::ostream& print_to_ostream(std::ostream& os, int indent = 0) const override {
        return os << std::string(indent, ' ') << "COUNT(" << (distinct ? "DISTINCT " : "") << "*)";
    }
};
} // namespace SPARQL