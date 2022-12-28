#pragma once

#include <memory>

#include "parser/query/expr/expr.h"

namespace SPARQL {
class ExprLessOrEqual : public Expr {
public:
    std::unique_ptr<Expr> lhs;
    std::unique_ptr<Expr> rhs;

    ExprLessOrEqual(std::unique_ptr<Expr> lhs, std::unique_ptr<Expr> rhs) :
        lhs (std::move(lhs)),
        rhs (std::move(rhs)) { }

    void accept_visitor(ExprVisitor& visitor) override {
        visitor.visit(*this);
    }

    virtual std::set<Var> get_vars() const override {
        std::set<Var> res = lhs->get_vars();
        auto rhs_vars = rhs->get_vars();
        res.insert(rhs_vars.begin(), rhs_vars.end());
        return res;
    }

    virtual std::ostream& print_to_ostream(std::ostream& os, int indent = 0) const override {
        return os << std::string(indent, ' ') << '(' << *lhs << " <= " << *rhs << ')';
    }
};
} // namespace SPARQL
