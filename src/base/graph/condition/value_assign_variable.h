#ifndef BASE__VALUE_ASSIGN_VARIABLE_H_
#define BASE__VALUE_ASSIGN_VARIABLE_H_

#include "base/graph/condition/value_assign.h"
#include "base/parser/logical_plan/exceptions.h"

class ValueAssignVariable : public ValueAssign {
private:
    std::string var;
    std::string key;
public:
    ValueAssignVariable(std::string var, std::string key)
        : var(std::move(var)), key(std::move(key)) { }
    ~ValueAssignVariable() = default;

    std::shared_ptr<GraphObject> get_value(Binding& binding) {
        return binding.get(var, key);
    }

    void check_names(std::set<std::string>& node_names, std::set<std::string>& edge_names) {
        auto node_search = node_names.find(var);
        if (node_search == node_names.end()) {
            auto edge_search = edge_names.find(var);
            if (edge_search == edge_names.end()) {
                throw QuerySemanticException("Variable \"" + var + "\" used in WHERE is not declared in MATCH");
            }
        }
    }
};

#endif // BASE__VALUE_ASSIGN_VARIABLE_H_
