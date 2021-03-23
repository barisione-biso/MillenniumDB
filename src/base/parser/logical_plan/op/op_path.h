#ifndef BASE__OP_PATH_H_
#define BASE__OP_PATH_H_

#include "base/parser/logical_plan/op/op.h"

class OpPath : public Op {
public:
    virtual bool operator<(const OpPath&) const = 0;
    virtual bool nullable() const = 0;
};

#endif // BASE__OP_PATH_H_