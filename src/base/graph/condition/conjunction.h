#ifndef BASE__COJUNCTION_H_
#define BASE__COJUNCTION_H_

#include "base/graph/condition/condition.h"

#include <memory>
#include <vector>

class Conjunction : public Condition {
public:
    std::vector<std::unique_ptr<Condition>> conditions;

    Conjunction(std::unique_ptr<Condition> left, std::unique_ptr<Condition> right) {
        conditions.push_back(std::move(left));
        conditions.push_back(std::move(right));
    }

    bool eval() {
        for (auto& condition : conditions) {
            if (!condition->eval()) {
                return false;
            }
        }
        return true;
    }

    bool is_conjunction() {
        return true;
    }

    void add_to_conjunction(std::unique_ptr<Condition> condition) {
        conditions.push_back(std::move(condition));
    }

};

#endif //BASE__COJUNCTION_H_
