#ifndef BASE__OP_H_
#define BASE__OP_H_

#include <memory>
#include <string>

#include "base/parser/logical_plan/op/visitors/op_visitor.h"
#include "base/binding/binding.h"

class Op {
public:
    virtual ~Op() = default;

    virtual void accept_visitor(OpVisitor&) const = 0;
    // virtual void accept_visitor(OpVisitor&, NewVisitor&) const = 0;

    virtual std::ostream& print_to_ostream(std::ostream& os, int indent=0) const = 0;
    friend std::ostream& operator<<(std::ostream& os, const Op& b) {
        return b.print_to_ostream(os);
    }
};

#endif // BASE__OP_H_
