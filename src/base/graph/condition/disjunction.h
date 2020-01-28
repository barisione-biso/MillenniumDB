#ifndef BASE__DISJUNCTION_H_
#define BASE__DISJUNCTION_H_

#include "base/graph/condition/condition.h"

#include <memory>
#include <vector>

class Disjunction : public Condition {
public:
    std::unique_ptr<Condition> left;
    std::unique_ptr<Condition> right;

    Disjunction(std::unique_ptr<Condition> left, std::unique_ptr<Condition> right)
        : left(std::move(left)), right(std::move(right)) { }

    bool eval() {
        return left->eval() || right->eval();
    }

    bool is_conjunction() {
        return false;
    }

    void add_to_conjunction(std::unique_ptr<Condition>) {
        // TODO: throw error
    }

};

#endif //BASE__DISJUNCTION_H_
