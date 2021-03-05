#ifndef BASE__FORMULA_TO_CONDITION_H_
#define BASE__FORMULA_TO_CONDITION_H_

#include <map>
#include <memory>
#include <vector>

#include <boost/optional.hpp>
#include <boost/variant.hpp>

#include "base/graph/condition/comparisons/equals.h"
#include "base/graph/condition/comparisons/greater_or_equals.h"
#include "base/graph/condition/comparisons/greater_than.h"
#include "base/graph/condition/comparisons/less_or_equals.h"
#include "base/graph/condition/comparisons/less_than.h"
#include "base/graph/condition/comparisons/not_equals.h"
#include "base/graph/condition/conjunction.h"
#include "base/graph/condition/constant_condition.h"
#include "base/graph/condition/disjunction.h"
#include "base/graph/condition/negation.h"
#include "base/graph/graph_model.h"
#include "base/ids/object_id.h"

class Formula2ConditionVisitor : public boost::static_visitor<std::unique_ptr<Condition>> {
public:
    GraphModel& model;
    const std::map<std::string, VarId>& var_names2var_ids;
    std::map<VarId, std::pair<VarId, ObjectId>> property_map;
    std::size_t new_property_map_count;

    Formula2ConditionVisitor(GraphModel& model, const std::map<std::string, VarId>& var_names2var_ids);

    std::unique_ptr<Condition> operator()(query::ast::AtomicFormula const&);
    std::unique_ptr<Condition> operator()(query::ast::FormulaDisjunction const&);
    std::unique_ptr<Condition> operator()(query::ast::FormulaConjunction const&);
    std::unique_ptr<Condition> operator()(query::ast::Statement const&);

    std::unique_ptr<ValueAssign> get_value_assignator(boost::variant<query::ast::SelectItem, common::ast::Value> item);

};

#endif // BASE__FORMULA_TO_CONDITION_H_
