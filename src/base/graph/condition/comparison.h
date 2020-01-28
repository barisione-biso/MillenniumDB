#ifndef BASE__COMPARISON_H_
#define BASE__COMPARISON_H_

#include "base/parser/grammar/ast.h"
#include "base/graph/condition/condition.h"

#include <memory>
#include <vector>

class Comparison : public Condition {
public:

    Comparison(ast::Statement const& statement) {
        // TODO:
        auto left = statement.lhs;
        auto right = statement.rhs;
        auto op = statement.comparator;
     }

    bool eval() {
        // return left->eval() && right->eval();
        return true;
    }

    bool is_conjunction() {
        return false;
    }

    void add_to_conjunction(std::unique_ptr<Condition>) {
        // TODO: throw error
    }

};

#endif //BASE__COMPARISON_H_
