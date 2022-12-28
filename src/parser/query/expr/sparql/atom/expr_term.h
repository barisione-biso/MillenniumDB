#pragma once

#include "base/query/sparql/sparql_element.h"
#include "parser/query/expr/expr.h"

namespace SPARQL {
class ExprTerm : public Expr {
public:
    SparqlElement term;

    ExprTerm(SparqlElement term) : term (std::move(term)) { }

    void accept_visitor(ExprVisitor& visitor) override {
        visitor.visit(*this);
    }

    virtual std::set<Var> get_vars() const override {
        return std::set<Var>();
    }

    virtual std::ostream& print_to_ostream(std::ostream& os, int indent = 0) const override {
        return os << std::string(indent, ' ') << term;
    }
};
} // namespace SPARQL
