#pragma once

#include <memory>

#include "parser/query/expr/expr.h"

namespace SPARQL {
class ExprAggAvg : public Expr {
public:
    std::unique_ptr<Expr> expr;
    bool distinct;

    ExprAggAvg(std::unique_ptr<Expr> expr, bool distinct) :
        expr     (std::move(expr)),
        distinct (distinct) { }

    void accept_visitor(ExprVisitor& visitor) override {
        visitor.visit(*this);
    }

    virtual std::set<Var> get_vars() const override {
        return expr->get_vars();
    }

    virtual std::ostream& print_to_ostream(std::ostream& os, int indent = 0) const override {
        return os << std::string(indent, ' ') << "AVG(" << (distinct ? "DISTINCT " : "") << *expr << ')';
    }
};
} // namespace SPARQL