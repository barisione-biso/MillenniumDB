#pragma once

#include "parser/query/op/op.h"

namespace SPARQL {

class OpUnitTable : public Op {
public:
    void accept_visitor(OpVisitor& visitor) override {
        visitor.visit(*this);
    }

    std::set<Var> get_vars() const override {
        return std::set<Var>();
    }

    std::ostream& print_to_ostream(std::ostream& os, int indent = 0) const override {
        os << std::string(indent, ' ');
        os << "OpUnitTable()\n";
        return os;
    }
};
} // namespace SPARQL