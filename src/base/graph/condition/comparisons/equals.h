#ifndef BASE__EQUALS_H_
#define BASE__EQUALS_H_

#include "base/binding/binding.h"
#include "base/parser/grammar/ast.h"
#include "base/parser/grammar/value_visitor.h"
#include "base/graph/condition/comparisons/comparison.h"

#include <iostream>
#include <memory>

class Equals : public Comparison {
public:
    Equals(ast::Statement const& statement)
        : Comparison(statement) { }

    bool compare(GraphObject& lhs, GraphObject& rhs) override {
        return lhs == rhs;
    }

};

#endif //BASE__EQUALS_H_
