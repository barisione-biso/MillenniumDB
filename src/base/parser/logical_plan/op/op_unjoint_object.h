#ifndef BASE__OP_UNJOINT_OBJECT_H_
#define BASE__OP_UNJOINT_OBJECT_H_

#include <string>

#include "base/parser/logical_plan/op/op.h"

class OpUnjointObject : public Op {
public:
    const std::string obj_name;

    OpUnjointObject(std::string obj_name) :
        obj_name (std::move(obj_name )) { }

    ~OpUnjointObject() = default;

    void accept_visitor(OpVisitor& visitor) {
        visitor.visit(*this);
    }

    bool operator<(const OpUnjointObject& other) const {
        return obj_name < other.obj_name;
    }
};

#endif // BASE__OP_UNJOINT_OBJECT_H_
