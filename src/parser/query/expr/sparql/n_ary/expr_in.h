#pragma once

#include <memory>
#include <vector>

#include "parser/query/expr/expr.h"

namespace SPARQL {
class ExprIn : public Expr {
public:
    std::unique_ptr<Expr> lhs_expr;
    std::vector<std::unique_ptr<Expr>> exprs;

    ExprIn(std::unique_ptr<Expr> lhs_expr, std::vector<std::unique_ptr<Expr>> exprs) :
        lhs_expr  (std::move(lhs_expr)),
        exprs     (std::move(exprs)) { }

    void accept_visitor(ExprVisitor& visitor) override {
        visitor.visit(*this);
    }

    virtual std::set<Var> get_vars() const override {
        std::set<Var> res;

        for (auto& expr : exprs) {
            auto expr_vars = expr->get_vars();
            res.insert(expr_vars.begin(), expr_vars.end());
        }
        return res;
    }

    virtual std::ostream& print_to_ostream(std::ostream& os, int indent = 0) const override {
        os << std::string(indent, ' ') << *lhs_expr << " IN(";
        bool first = true;
        for (auto& expr : exprs) {
            if (!first) {
                os << ", ";
            }
            os << *expr;
            first = false;
        }
        return os << ')';
    }
};
} // namespace SPARQL
