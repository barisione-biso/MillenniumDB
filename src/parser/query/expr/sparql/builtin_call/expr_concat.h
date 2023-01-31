#pragma once

#include <memory>
#include <vector>

#include "parser/query/expr/expr.h"

namespace SPARQL {
class ExprConcat : public Expr {
public:
    std::vector<std::unique_ptr<Expr>> exprs;

    ExprConcat(std::vector<std::unique_ptr<Expr>> exprs) :
        exprs (std::move(exprs)) { }

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
        os << std::string(indent, ' ') << "CONCAT(";
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
