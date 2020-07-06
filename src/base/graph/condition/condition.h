#ifndef BASE__CONDITION_H_
#define BASE__CONDITION_H_

#include <set>

#include "base/graph/graph_object.h"

class Binding;

enum class ConditionType {
    comparison,
    conjunction,
    constant,
    disjunction,
    negation
};

// Abstract class
class Condition {
public:
    virtual ~Condition() { };
    virtual bool eval(Binding&) = 0;
    virtual ConditionType type() = 0;
    virtual void check_names(std::set<std::string>& node_names, std::set<std::string>& edge_names) = 0;
};

#endif // BASE__CONDITION_H_
