#pragma once

#include "parser/query/op/op.h"
#include "parser/query/op/sparql/op_triple.h"
#include "parser/query/op/sparql/op_path.h"

namespace SPARQL {

class OpBasicGraphPattern : public Op {
public:
    std::vector<OpTriple> triples;
    std::vector<OpPath>   paths;

    OpBasicGraphPattern(std::vector<OpTriple>&& triples, std::vector<OpPath>&& paths) :
        triples(std::move(triples)), paths(std::move(paths)) { }

    void accept_visitor(OpVisitor& visitor) override {
        visitor.visit(*this);
    }

    std::set<Var> get_vars() const override {
        std::set<Var> res;
        for (auto& triple : triples) {
            auto vars = triple.get_vars();
            res.insert(vars.begin(), vars.end());
        }
        for (auto& path : paths) {
            auto vars = path.get_vars();
            res.insert(vars.begin(), vars.end());
        }
        return res;
    }

    std::ostream& print_to_ostream(std::ostream& os, int indent = 0) const override {
        os << std::string(indent, ' ');
        os << "OpBasicGraphPattern(\n";
        for (auto& triple : triples) {
            os << std::string(indent + 2, ' ');
            os << triple;
        }
        for (auto& path : paths) {
            os << std::string(indent + 2, ' ');
            os << path;
        }
        os << std::string(indent, ' ');
        os << ")\n";
        return os;
    }
};
} // namespace SPARQL