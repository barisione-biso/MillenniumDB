#pragma once

#include "base/query/var.h"
#include "parser/query/expr/expr.h"

namespace SPARQL {
class ExprVar : public Expr {
public:
    Var var;

    ExprVar(const std::string& var) :
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
        return os << std::string(indent, ' ') << '?' << var;
    }
};
} // namespace SPARQL
