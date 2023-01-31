#pragma once

#include <memory>

#include "parser/query/expr/expr.h"

namespace SPARQL {

class ExprRegex : public Expr {
public:
    std::unique_ptr<Expr> expr1;
    std::unique_ptr<Expr> expr2;

    // may be nullptr
    std::unique_ptr<Expr> expr3;

    ExprRegex(std::unique_ptr<Expr> expr1,
              std::unique_ptr<Expr> expr2) :
        expr1 (std::move(expr1)),
        expr2 (std::move(expr2)) { }

    ExprRegex(std::unique_ptr<Expr> expr1,
              std::unique_ptr<Expr> expr2,
              std::unique_ptr<Expr> expr3) :
        expr1 (std::move(expr1)),
        expr2 (std::move(expr2)),
        expr3 (std::move(expr3)) { }

    void accept_visitor(ExprVisitor& visitor) override {
        visitor.visit(*this);
    }

    virtual std::set<Var> get_vars() const override {
        std::set<Var> res;
        auto expr1_vars = expr1->get_vars();
        auto expr2_vars = expr2->get_vars();
        res.insert(expr1_vars.begin(), expr1_vars.end());
        res.insert(expr2_vars.begin(), expr2_vars.end());
        if (expr3 != nullptr) {
            auto expr3_vars = expr3->get_vars();
            res.insert(expr3_vars.begin(), expr3_vars.end());
        }
        return res;
    }

    virtual std::ostream& print_to_ostream(std::ostream& os, int indent = 0) const override {
        os << std::string(indent, ' ') << "REGEX("
                  << *expr1 << ", "  << *expr2;
        if (expr3 != nullptr) {
            os << ", " << *expr3;
        }

        return os << ')';
    }
};
} // namespace SPARQL
