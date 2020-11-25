#ifndef BASE__NOT_EQUALS_H_
#define BASE__NOT_EQUALS_H_

#include "base/graph/condition/comparisons/comparison.h"

class NotEquals : public Comparison {
public:
    NotEquals(std::unique_ptr<ValueAssign> lhs, std::unique_ptr<ValueAssign> rhs) :
        Comparison(std::move(lhs), std::move(rhs)) { }

    bool compare(GraphObject& lhs, GraphObject& rhs) override {
        return lhs != rhs;
    }
};

#endif // BASE__NOT_EQUALS_H_
