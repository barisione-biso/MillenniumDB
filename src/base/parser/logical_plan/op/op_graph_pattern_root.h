#ifndef BASE__OP_GRAPH_PATTERN_ROOT_H_
#define BASE__OP_GRAPH_PATTERN_ROOT_H_

#include <string>
#include <vector>

#include "base/parser/grammar/query/query_ast.h"
#include "base/parser/logical_plan/op/op.h"

class OpGraphPatternRoot : public Op {
public:
    std::unique_ptr<Op> op;

    OpGraphPatternRoot(std::unique_ptr<Op> op) :
        op (std::move(op)) { }

    ~OpGraphPatternRoot() = default;

    std::ostream& print_to_ostream(std::ostream& os, int indent=0) const override {
        os << std::string(indent, ' ');
        os << "OpGraphPatternRoot()";
        os << "\n";
        return op->print_to_ostream(os, indent + 2);
    };

    void accept_visitor(OpVisitor& visitor) override {
        visitor.visit(*this);
    }

    void get_vars(std::set<Var>& set) const override {
        op->get_vars(set);
    }
};

#endif // BASE__OP_GRAPH_PATTERN_ROOT_H_
