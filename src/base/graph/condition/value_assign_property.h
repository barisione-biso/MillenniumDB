#ifndef BASE__VALUE_ASSIGN_PROPERTY_H_
#define BASE__VALUE_ASSIGN_PROPERTY_H_

#include "base/graph/condition/value_assign.h"
#include "base/parser/logical_plan/exceptions.h"

class ValueAssignProperty : public ValueAssign {
private:
    std::string var;
    std::string key;

public:
    ValueAssignProperty(std::string var, std::string key)
        : var(std::move(var)), key(std::move(key)) { }
    ~ValueAssignProperty() = default;

    std::shared_ptr<GraphObject> get_value(Binding& binding) {
        return binding.get(var, key);
    }

    void check_names(std::set<std::string>& var_names) {
        if (var_names.find(var) == var_names.end()) {
            throw QuerySemanticException("Variable \"" + var + "\" used in WHERE is not declared in MATCH");
        }
    }
};

#endif // BASE__VALUE_ASSIGN_PROPERTY_H_
