#pragma once

#include <memory>

#include "parser/query/expr/expr.h"

namespace SPARQL {
class ExprBound : public Expr {
public:
    Var var;

    ExprBound(Var&& var) :
        var (std::move(var)) { }

    ExprBound(const Var& var) :
        var (var) { }

    void accept_visitor(ExprVisitor& visitor) override {
        visitor.visit(*this);
    }

    virtual std::set<Var> get_vars() const override {
        std::set<Var> res;
        res.insert(var);
        return res;
    }

    virtual std::ostream& print_to_ostream(std::ostream& os, int indent = 0) const override {
        return os << std::string(indent, ' ') << "BOUND(" << var << ')';
    }
};
} // namespace SPARQL
