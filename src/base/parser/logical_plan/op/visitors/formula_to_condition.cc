#include "formula_to_condition.h"

Formula2ConditionVisitor::Formula2ConditionVisitor(GraphModel& model, const std::map<std::string, VarId>& var_names2var_ids) :
    model                  (model),
    var_names2var_ids      (var_names2var_ids),
    new_property_map_count (var_names2var_ids.size()) { }


std::unique_ptr<Condition> Formula2ConditionVisitor::operator()(query::ast::Formula const& formula) {
    std::vector<std::unique_ptr<Condition>> tmp_disjunction;
    auto tmp = (*this)(formula.root);

    for (auto const& step_formula : formula.path) {
        auto step = (*this)(step_formula.condition);
        if (step_formula.op == query::ast::BinaryOp::Or) { // OR
            tmp_disjunction.push_back(std::move(tmp));
            tmp = std::move(step);
        }
        else { // AND
            if (tmp->type() == ConditionType::conjunction) { // already AND
                Conjunction* conjuction = static_cast<Conjunction*>(tmp.get());
                conjuction->add(std::move(step));
            }
            else {
                auto tmp2 = std::make_unique<Conjunction>();
                tmp2->add(std::move(tmp));
                tmp2->add(std::move(step));
                tmp = std::move(tmp2);
            }
        }
    }
    if (tmp_disjunction.size() > 0) {
        auto res = std::make_unique<Disjunction>(std::move(tmp_disjunction));
        res->add(std::move(tmp));
        return res;
    }
    else return tmp;
}


std::unique_ptr<Condition> Formula2ConditionVisitor::operator()(query::ast::Condition const& condition) {
    if (condition.negation) {
        std::unique_ptr<Condition> cont = boost::apply_visitor(*this, condition.content);
        return std::make_unique<Negation>(std::move(cont));
    }
    else {
        return boost::apply_visitor(*this, condition.content);
    }
}


std::unique_ptr<ValueAssign> Formula2ConditionVisitor::get_value_assignator(
    boost::variant<query::ast::SelectItem, common::ast::Value> item)
{
    if (item.type() == typeid(query::ast::SelectItem)) {
        auto select_item = boost::get<query::ast::SelectItem>(item);
        auto find_var_id = var_names2var_ids.find(select_item.var);
        assert(find_var_id != var_names2var_ids.end()
                && "Variable names inside WHERE need to be checked before processing conditions");

        if (select_item.key) {
            VarId new_property_var_id(new_property_map_count++);
            auto property_key_object_id = model.get_string_id(select_item.key.get());
            property_map.insert({ new_property_var_id, std::make_pair(find_var_id->second, property_key_object_id) });
            return std::make_unique<ValueAssignVariable>(new_property_var_id);
        } else {
            return std::make_unique<ValueAssignVariable>(find_var_id->second);
        }
    } else {
        auto casted_value = boost::get<query::ast::Value>(item);
        if (casted_value.type() == typeid(std::string)) {
            // strings can be destroyed after `casted_value` is visited, so it needs to be handled differently
            auto str_ptr = std::make_unique<std::string>(boost::get<std::string>(casted_value));
            auto str_graph_object = GraphObject::make_string(str_ptr->c_str());
            return std::make_unique<ValueAssignConstant>(str_graph_object, move(str_ptr));
        } else {
            auto visitor = ValueVisitor();
            auto value = visitor(casted_value);
            return std::make_unique<ValueAssignConstant>(value, nullptr);
        }
    }
}


std::unique_ptr<Condition> Formula2ConditionVisitor::operator()(query::ast::Statement const& statement) {
    std::unique_ptr<ValueAssign> lhs = get_value_assignator(statement.lhs);
    std::unique_ptr<ValueAssign> rhs = get_value_assignator(statement.rhs);

    switch (statement.comparator) {
        case query::ast::Comparator::EQ:
            return std::make_unique<Equals>(std::move(lhs), std::move(rhs));

        case query::ast::Comparator::NE:
            return std::make_unique<NotEquals>(std::move(lhs), std::move(rhs));

        case query::ast::Comparator::LE:
            return std::make_unique<LessOrEquals>(std::move(lhs), std::move(rhs));

        case query::ast::Comparator::GE:
            return std::make_unique<GreaterOrEquals>(std::move(lhs), std::move(rhs));

        case query::ast::Comparator::GT:
            return std::make_unique<GreaterThan>(std::move(lhs), std::move(rhs));

        case query::ast::Comparator::LT:
            return std::make_unique<LessThan>(std::move(lhs), std::move(rhs));

        default:
            return nullptr;
    }
}
