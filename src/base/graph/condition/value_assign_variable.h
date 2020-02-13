#ifndef BASE__VALUE_ASSIGN_VARIABLE_H_
#define BASE__VALUE_ASSIGN_VARIABLE_H_

#include "base/graph/condition/value_assign.h"

class ValueAssignVariable : public ValueAssign {
private:
    std::shared_ptr<Value> value;
public:
    ValueAssignConstant(std::shared_ptr<Value> value)
        : value(value) { }
    ~ValueAssignConstant() = default;

    std::shared_ptr<Value> get_value(Binding&) {
        return value;
    }
};

#endif //BASE__VALUE_ASSIGN_VARIABLE_H_
