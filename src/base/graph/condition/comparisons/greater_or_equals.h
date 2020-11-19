#ifndef BASE__GREATER_OR_EQUALS_H_
#define BASE__GREATER_OR_EQUALS_H_

#include "base/graph/condition/comparisons/comparison.h"

class GreaterOrEquals : public Comparison {
public:
    GreaterOrEquals(std::unique_ptr<ValueAssign> lhs, std::unique_ptr<ValueAssign> rhs) :
        Comparison(std::move(lhs), std::move(rhs)) { }

    bool compare(GraphObject& lhs, GraphObject& rhs) override {
        return lhs >= rhs;
    }
};

#endif // BASE__GREATER_OR_EQUALS_H_
