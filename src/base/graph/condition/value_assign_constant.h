#ifndef BASE__VALUE_ASSIGN_CONSTANT_H_
#define BASE__VALUE_ASSIGN_CONSTANT_H_

#include "base/graph/condition/value_assign.h"

class ValueAssignConstant : public ValueAssign {
private:
    std::string var;
    std::string key;
public:
    ValueAssignConstant(std::string var, std::string key)
        : var(std::move(var)), key(std::move(key)) { }
    ~ValueAssignConstant() = default;

    std::shared_ptr<Value> get_value(Binding& binding) {
        return binding.get(var, key);
    }
};

#endif //BASE__VALUE_ASSIGN_CONSTANT_H_
