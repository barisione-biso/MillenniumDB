#ifndef BASE__OP_H_
#define BASE__OP_H_

#include <string>
#include <memory>

#include "base/parser/logical_plan/op/visitors/op_visitor.h"

class Op {
public:
    virtual ~Op() = default;
    virtual void accept_visitor(OpVisitor&) const = 0;
};

#endif // BASE__OP_H_
