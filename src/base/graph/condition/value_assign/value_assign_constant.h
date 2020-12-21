#ifndef BASE__VALUE_ASSIGN_CONSTANT_H_
#define BASE__VALUE_ASSIGN_CONSTANT_H_

#include "base/graph/condition/value_assign/value_assign.h"
#include "base/graph/graph_object.h"

class ValueAssignConstant : public ValueAssign {
private:
    GraphObject value;

public:
    ValueAssignConstant(GraphObject value)
        : value(value) { }

    ~ValueAssignConstant() = default;

    GraphObject get_value(Binding&) {
        return value;
    }
};

#endif // BASE__VALUE_ASSIGN_CONSTANT_H_
