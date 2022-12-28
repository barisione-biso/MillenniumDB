#pragma once

#include <memory>

#include "parser/query/expr/expr.h"

namespace SPARQL {
class ExprSHA384 : public Expr {
public:
    std::unique_ptr<Expr> expr;

    ExprSHA384(std::unique_ptr<Expr> expr) :
        expr (std::move(expr)) { }

    void accept_visitor(ExprVisitor& visitor) override {
        visitor.visit(*this);
    }

    virtual std::set<Var> get_vars() const override {
        return expr->get_vars();
    }

    virtual std::ostream& print_to_ostream(std::ostream& os, int indent = 0) const override {
        return os << std::string(indent, ' ') << "SHA384(" << *expr << ')';
    }
};
} // namespace SPARQL
