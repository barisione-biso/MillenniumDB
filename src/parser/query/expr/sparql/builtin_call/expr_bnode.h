#pragma once

#include <memory>

#include "parser/query/expr/expr.h"

namespace SPARQL {
class ExprBNode : public Expr {
public:
    // may be nullptr
    std::unique_ptr<Expr> expr;

    ExprBNode(std::unique_ptr<Expr> expr) :
        expr (std::move(expr)) { }

    void accept_visitor(ExprVisitor& visitor) override {
        visitor.visit(*this);
    }

    virtual std::set<Var> get_vars() const override {
        return expr->get_vars();
    }

    virtual std::ostream& print_to_ostream(std::ostream& os, int indent = 0) const override {
        os << std::string(indent, ' ') << "BNODE(";
        if (expr != nullptr) {
            os << *expr;
        }
        os << ')';
        return os;
    }
};
} // namespace SPARQL
