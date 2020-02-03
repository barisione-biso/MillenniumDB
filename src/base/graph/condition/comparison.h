#ifndef BASE__COMPARISON_H_
#define BASE__COMPARISON_H_

#include "base/parser/grammar/ast.h"
#include "base/graph/condition/condition.h"

class Comparison : public Condition {
public:

    Comparison(ast::Statement const& statement) {
        // TODO:
        // auto left = statement.lhs;
        // auto right = statement.rhs;
        // auto op = statement.comparator;
     }

    bool eval(Binding&) {
        // TODO:
        return true;
    }

    ConditionType type() {
        return ConditionType::comparison;
    }
};

#endif //BASE__COMPARISON_H_
