#pragma once

#include <memory>

#include "parser/query/expr/expr.h"

namespace SPARQL {

/* https://www.w3.org/TR/sparql11-query/#defn_aggSample
Definition: Sample
RDFTerm Sample(multiset M)

Sample(M) = v, where v in Flatten(M)

Sample({}) = error
*/

// Sample is a set function which returns an arbitrary value from the multiset
// passed to it.
class ExprAggSample : public Expr {
public:
    std::unique_ptr<Expr> expr;
    bool distinct;

    ExprAggSample(std::unique_ptr<Expr> expr, bool distinct) :
        expr     (std::move(expr)),
        distinct (distinct) { }

    void accept_visitor(ExprVisitor& visitor) override {
        visitor.visit(*this);
    }

    virtual std::set<Var> get_vars() const override {
        return expr->get_vars();
    }

    virtual std::ostream& print_to_ostream(std::ostream& os, int indent = 0) const override {
        return os << std::string(indent, ' ') << "SAMPLE(" << (distinct ? "DISTINCT " : "") << *expr << ")";
    }
};
} // namespace SPARQL
