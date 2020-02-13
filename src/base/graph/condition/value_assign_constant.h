#ifndef BASE__VALUE_ASSIGN_CONSTANT_H_
#define BASE__VALUE_ASSIGN_CONSTANT_H_

#include "base/graph/condition/value_assign.h"

class ValueAssignConstant : public ValueAssign {
private:
    std::unique_ptr<Value> value;
public:
    ValueAssignConstant(std::unique_ptr<Value> value)
        : value(std::move(value)) { }
    ~ValueAssignConstant() = default;

    std::shared_ptr<Value> get_value(Binding&) {
        return std::make_shared<Value>(value);
    }
};

#endif //BASE__VALUE_ASSIGN_CONSTANT_H_
