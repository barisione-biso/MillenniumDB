#ifndef BASE__NEGATION_H_
#define BASE__NEGATION_H_

#include "base/graph/condition/condition.h"

#include <memory>
#include <vector>

class Negation : public Condition {
public:
    std::unique_ptr<Condition> condition;

    Negation(std::unique_ptr<Condition> condition)
        : condition(std::move(condition)) { }

    bool eval() {
        return !condition->eval();
    }

    bool is_conjunction() {
        return false;
    }

    void add_to_conjunction(std::unique_ptr<Condition>) {
        // TODO: throw error
    }
};

#endif //BASE__NEGATION_H_
