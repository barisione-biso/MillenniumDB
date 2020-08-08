#ifndef BASE__DISJUNCTION_H_
#define BASE__DISJUNCTION_H_

#include <memory>
#include <vector>

#include "base/graph/condition/condition.h"

class Disjunction : public Condition {
public:
    std::vector<std::unique_ptr<Condition>> conditions;

    Disjunction() { }
    Disjunction(std::vector<std::unique_ptr<Condition>> conditions)
        : conditions (std::move(conditions)) { }

    bool eval(Binding& binding) {
        for (auto& condition : conditions) {
            if (condition->eval(binding)) {
                return true;
            }
        }
        return false;
    }

    ConditionType type() {
        return ConditionType::disjunction;
    }

    void add(std::unique_ptr<Condition> condition) {
        conditions.push_back(std::move(condition));
    }

    void check_names(std::set<std::string>& node_names, std::set<std::string>& edge_names) {
        for (auto& condition : conditions) {
            condition->check_names(node_names, edge_names);
        }
    }
};

#endif // BASE__DISJUNCTION_H_
