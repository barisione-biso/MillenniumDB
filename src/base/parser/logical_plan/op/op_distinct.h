#ifndef BASE__OP_DISTINCT_H_
#define BASE__OP_DISTINCT_H_

#include <string>
#include <vector>

#include "base/parser/logical_plan/op/op.h"

class OpDistinct : public Op {
public:
    const std::unique_ptr<Op> op;

    OpDistinct(std::unique_ptr<Op> op) :
        op (std::move(op))
        { }

    ~OpDistinct() = default;

    void accept_visitor(OpVisitor& visitor) override {
        visitor.visit(*this);
    }

    std::set<std::string> get_var_names() const override {
        return op->get_var_names();
    }

    std::ostream& print_to_ostream(std::ostream& os, int indent=0) const override {
        os << std::string(indent, ' ');
        os << "OpDistinct()\n";
        return op->print_to_ostream(os, indent + 2);
    }
};

#endif // BASE__OP_DISTINCT_H_
