#pragma once

#include <vector>

#include "base/query/sparql/sparql_element.h"
#include "parser/query/expr/expr.h"
#include "parser/query/op/op.h"

namespace SPARQL {

class OpFilter : public Op {
public:
    std::unique_ptr<Op> op;

    std::vector<std::unique_ptr<Expr>> filters;

    OpFilter(std::unique_ptr<Op> op, std::vector<std::unique_ptr<Expr>> filters) :
        op      (std::move(op)),
        filters (std::move(filters)) { }

    void accept_visitor(OpVisitor& visitor) override {
        visitor.visit(*this);
    }

    std::set<Var> get_vars() const override {
        return op->get_vars();
    }

    std::ostream& print_to_ostream(std::ostream& os, int indent = 0) const override {
        os << std::string(indent, ' ') << "OpFilter(\n";

        filters[0]->print_to_ostream(os, indent + 2);
        for (size_t i = 1; i < filters.size(); i++) {
            os << ",\n";
            filters[i]->print_to_ostream(os, indent + 2);
        }
        os << "\n";
        op->print_to_ostream(os, indent+2);
        os << std::string(indent, ' ') << ")\n";
        return os;
    }
};
} // namespace SPARQL
