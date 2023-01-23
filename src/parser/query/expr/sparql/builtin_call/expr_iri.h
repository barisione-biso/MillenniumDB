#pragma once

#include <memory>

#include "parser/query/expr/expr.h"

namespace SPARQL {
class ExprIRI : public Expr {
public:
    std::unique_ptr<Expr> expr;
    std::string base_iri;

    ExprIRI(std::unique_ptr<Expr> expr, std::string base_iri) :
        expr (std::move(expr)), base_iri (base_iri) { }

    void accept_visitor(ExprVisitor& visitor) override {
        visitor.visit(*this);
    }

    virtual std::set<Var> get_vars() const override {
        return expr->get_vars();
    }

    virtual std::ostream& print_to_ostream(std::ostream& os, int indent = 0) const override {
        return os << std::string(indent, ' ') << "IRI(" << *expr << ')';
    }
};
} // namespace SPARQL
