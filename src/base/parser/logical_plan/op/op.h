#ifndef BASE__OP_H_
#define BASE__OP_H_

#include <set>

#include "base/parser/logical_plan/op/visitors/op_visitor.h"
#include "base/parser/logical_plan/var.h"

class Op {
public:
    virtual ~Op() = default;

    virtual void accept_visitor(OpVisitor&) = 0;

    // OpFilter, OpGroupBy and OpOrderBy are not adding their new variables (properties)
    virtual void get_vars(std::set<Var>& set) const = 0;

    virtual std::ostream& print_to_ostream(std::ostream& os, int indent=0) const = 0;

    friend std::ostream& operator<<(std::ostream& os, const Op& b) {
        return b.print_to_ostream(os);
    }
};

#endif // BASE__OP_H_
