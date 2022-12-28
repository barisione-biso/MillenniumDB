#pragma once

#include <memory>

#include "parser/query/expr/expr.h"
#include "parser/query/op/op.h"

namespace SPARQL {
class ExprNotExists : public Expr {
public:
    std::unique_ptr<Op> op;

    ExprNotExists(std::unique_ptr<Op> op) :
        op (std::move(op)) { }

    void accept_visitor(ExprVisitor& visitor) override {
        visitor.visit(*this);
    }

    virtual std::set<Var> get_vars() const override {
        return op->get_vars(); // TODO: do we want this? or only scoped vars?
    }

    virtual std::ostream& print_to_ostream(std::ostream& os, int indent = 0) const override {
        return os << std::string(indent, ' ') << "NOT EXISTS(" << *op << ')';
    }
};
} // namespace SPARQL
