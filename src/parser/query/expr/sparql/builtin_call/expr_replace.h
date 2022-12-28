#pragma once

#include <memory>

#include "parser/query/expr/expr.h"

namespace SPARQL {

class ExprReplace : public Expr {
public:
    std::unique_ptr<Expr> expr1;
    std::unique_ptr<Expr> expr2;
    std::unique_ptr<Expr> expr3;

    // may be nullptr
    std::unique_ptr<Expr> expr4;

    ExprReplace(std::unique_ptr<Expr> expr1,
                std::unique_ptr<Expr> expr2,
                std::unique_ptr<Expr> expr3) :
        expr1 (std::move(expr1)),
        expr2 (std::move(expr2)),
        expr3 (std::move(expr3)) { }

    ExprReplace(std::unique_ptr<Expr> expr1,
                std::unique_ptr<Expr> expr2,
                std::unique_ptr<Expr> expr3,
                std::unique_ptr<Expr> expr4) :
        expr1 (std::move(expr1)),
        expr2 (std::move(expr2)),
        expr3 (std::move(expr3)),
        expr4 (std::move(expr4)) { }

    void accept_visitor(ExprVisitor& visitor) override {
        visitor.visit(*this);
    }

    virtual std::set<Var> get_vars() const override {
        std::set<Var> res;
        auto expr1_vars = expr1->get_vars();
        auto expr2_vars = expr2->get_vars();
        auto expr3_vars = expr3->get_vars();
        res.insert(expr1_vars.begin(), expr1_vars.end());
        res.insert(expr2_vars.begin(), expr2_vars.end());
        res.insert(expr3_vars.begin(), expr3_vars.end());

        if (expr4 != nullptr) {
            auto expr4_vars = expr4->get_vars();
            res.insert(expr4_vars.begin(), expr4_vars.end());
        }
        return res;
    }

    virtual std::ostream& print_to_ostream(std::ostream& os, int indent = 0) const override {
        os << std::string(indent, ' ') << "REPLACE("
           << *expr1 << ", "  << *expr2 << ", " << *expr3;
        if (expr4 != nullptr) {
            os << ", " << *expr4;
        }
        return os << ')';
    }
};
} // namespace SPARQL
