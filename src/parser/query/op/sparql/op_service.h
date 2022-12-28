#pragma once

#include <memory>
#include <set>
#include <variant>
#include <vector>

#include "base/query/sparql/iri.h"
#include "base/query/var.h"
#include "parser/query/op/op.h"

namespace SPARQL {

class OpService : public Op {
public:
    bool silent;
    std::variant<Var, Iri> graph;
    std::unique_ptr<Op> op;

    OpService(bool silent, Var var, std::unique_ptr<Op> op) :
        silent(silent), graph(var), op(std::move(op)) { }

    OpService(bool silent, Iri iri, std::unique_ptr<Op> op) :
        silent(silent), graph(iri), op(std::move(op)) { }

    void accept_visitor(OpVisitor& visitor) override {
        visitor.visit(*this);
    }

    std::set<Var> get_vars() const override {
        return op->get_vars();
    }

    std::ostream& print_to_ostream(std::ostream& os, int indent = 0) const override {
        os << std::string(indent, ' ');
        os << "OpService(";
        if (silent) {
            os << "SILENT ";
        }
        if (std::holds_alternative<Var>(graph)) {
            os << std::get<Var>(graph) << '\n';
        } else {
            os << '<' << std::get<Iri>(graph).name << ">\n";
        }
        op->print_to_ostream(os, indent + 2);
        os << std::string(indent, ' ');
        os << ")\n";
        return os;
    }
};
} // namespace SPARQL