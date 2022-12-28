#pragma once

#include <memory>

#include "parser/query/expr/expr.h"

namespace SPARQL {
class ExprStrLang : public Expr {
public:
    std::unique_ptr<Expr> expr1;
    std::unique_ptr<Expr> expr2;

    ExprStrLang(std::unique_ptr<Expr> expr1, std::unique_ptr<Expr> expr2) :
        expr1 (std::move(expr1)),
        expr2 (std::move(expr2)) { }

    void accept_visitor(ExprVisitor& visitor) override {
        visitor.visit(*this);
    }

    virtual std::set<Var> get_vars() const override {
        auto expr1_vars = expr1->get_vars();
        auto expr2_vars = expr2->get_vars();
        expr1_vars.insert(expr2_vars.begin(), expr2_vars.end());
        return expr1_vars;
    }

    virtual std::ostream& print_to_ostream(std::ostream& os, int indent = 0) const override {
        return os << std::string(indent, ' ') << "STRENDS(" << *expr1 << ", " << *expr2 << ')';
    }
};
} // namespace SPARQL
