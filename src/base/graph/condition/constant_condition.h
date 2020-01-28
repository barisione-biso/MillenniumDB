#ifndef BASE__CONSTANT_CONDITION_H_
#define BASE__CONSTANT_CONDITION_H_

#include "base/graph/condition/condition.h"

class ConstantCondition : public Condition {
public:
    const bool constant;

    ConstantCondition (bool constant) : constant(constant) { }

    bool eval() {
        return constant;
    }

    bool is_conjunction() {
        return false;
    }

    void add_to_conjunction(std::unique_ptr<Condition>) {
        // TODO: throw error
    }
};

#endif //BASE__CONSTANT_CONDITION_H_
