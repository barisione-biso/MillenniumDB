#ifndef BASE__VALUE_ASSIGN_VARIABLE_H_
#define BASE__VALUE_ASSIGN_VARIABLE_H_

#include "base/graph/condition/value_assign.h"
#include "base/parser/logical_plan/exceptions.h"

class ValueAssignVariable : public ValueAssign {
private:
    std::string var;

public:
    ValueAssignVariable(std::string var) :
        var(std::move(var)) { }
    ~ValueAssignVariable() = default;

    std::shared_ptr<GraphObject> get_value(Binding& binding) {
        return binding[var];
    }

    void check_names(std::set<std::string>& var_names) {
        if (var_names.find(var) == var_names.end()) {
            throw QuerySemanticException("Variable \"" + var + "\" used in WHERE is not declared in MATCH");
        }
    }
};

#endif // BASE__VALUE_ASSIGN_VARIABLE_H_
