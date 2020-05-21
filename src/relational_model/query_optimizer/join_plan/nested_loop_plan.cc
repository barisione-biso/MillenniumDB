#include "nested_loop_plan.h"

#include "relational_model/physical_plan/binding_id_iter/index_nested_loop_join.h"

using namespace std;

NestedLoopPlan::NestedLoopPlan(unique_ptr<JoinPlan> _lhs, unique_ptr<JoinPlan> _rhs) :
    lhs(move(_lhs)), rhs(move(_rhs))
{
    auto var_order = lhs->get_var_order();
    rhs->set_input_vars(var_order);
}


NestedLoopPlan::NestedLoopPlan(const NestedLoopPlan& other) :
    lhs(other.lhs->duplicate()), rhs(other.rhs->duplicate()) { }


unique_ptr<JoinPlan> NestedLoopPlan::duplicate() {
    return make_unique<NestedLoopPlan>(*this);
}


void NestedLoopPlan::print() {
    cout << "NestedLoopPlan(";
    lhs->print();
    cout << ",";
    rhs->print();
    cout << ")";
}


double NestedLoopPlan::estimate_cost() {
    return estimate_cost(*lhs, *rhs);
}


double NestedLoopPlan::estimate_cost(JoinPlan& lhs, JoinPlan& rhs) {
    return lhs.estimate_cost() + (lhs.estimate_output_size() * rhs.estimate_cost());
}


double NestedLoopPlan::estimate_output_size() {
    // TODO:
    return lhs->estimate_output_size() * rhs->estimate_output_size();
}


bool NestedLoopPlan::cartesian_product_needed(JoinPlan& other) {
    for (auto& my_var : get_var_order()) {
        for (auto& other_var : other.get_var_order()) {
            if (my_var == other_var) {
                return false;
            }
        }
    }
    return true;
}


vector<VarId> NestedLoopPlan::get_var_order() {
    auto result = lhs->get_var_order();

    for (auto& right_var : rhs->get_var_order()) {
        bool already_present = false;
        for (auto res_var : result) {
            if (res_var == right_var) {
                already_present = true;
                break;
            }
        }
        if (!already_present) {
            result.push_back(right_var);
        }
    }
    return result;
}


void NestedLoopPlan::set_input_vars(std::vector<VarId>& input_var_order) {
    lhs->set_input_vars(input_var_order);
    auto left_var_order = lhs->get_var_order();
    rhs->set_input_vars(left_var_order);
}


unique_ptr<BindingIdIter> NestedLoopPlan::get_binding_id_iter() {
    return make_unique<IndexNestedLoopJoin>(lhs->get_binding_id_iter(), rhs->get_binding_id_iter());
}