#ifndef BASE__EQUALS_H_
#define BASE__EQUALS_H_

#include "base/parser/grammar/ast.h"
#include "base/parser/grammar/value_visitor.h"
#include "base/graph/condition/condition.h"
#include "base/graph/value/value.h"
#include "base/binding/binding.h"

#include <iostream>
#include <memory>

class Equals : public Condition {
public:
    // const ast::Statement& statement;
    std::string left_name;
    std::string left_var;
    std::string left_key;

    // only one of the two is valid. If rhs != null, right_name should be an empty string.
    std::unique_ptr<Value> rhs;
    std::string right_name;
    std::string right_var;
    std::string right_key;

    Equals(ast::Statement const& statement) {
        left_var = statement.lhs.variable;
        left_key = statement.lhs.key;
        left_name = left_var + "." + left_key;
    
        if (statement.rhs.type() == typeid(ast::Element)) {
            rhs = nullptr;
            auto right = boost::get<ast::Element>(statement.rhs);
            right_var = right.variable;
            right_key = right.variable;
            right_name = right_var + "." + right_key;
        }
        else {
            auto right = boost::get<ast::Value>(statement.rhs);
            auto visitor = ValueVisitor();
            rhs = visitor(right);
        }
     }

    bool eval(Binding& binding) {
        auto lhs = binding[left_name];
        if (lhs == nullptr) {
            // TODO: make join? necesitaria
            binding.print();
            lhs = binding.try_extend(left_var, left_key);
        }

        if (rhs != nullptr) { // If the right element is a constant value
            return *lhs == *rhs;
        }
        else {
            auto right = binding[right_name];
            return *lhs == *right;
        }
    }

    ConditionType type() {
        return ConditionType::comparison;
    }
};

#endif //BASE__EQUALS_H_ 
