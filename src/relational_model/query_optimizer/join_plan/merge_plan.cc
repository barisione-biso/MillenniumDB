#include "merge_plan.h"

#include <limits>

#include "relational_model/physical_plan/binding_id_iter/merge_join.h"

using namespace std;

MergePlan::MergePlan(unique_ptr<JoinPlan> lhs, unique_ptr<JoinPlan> rhs) :
    lhs(move(lhs)), rhs(move(rhs)) { }


MergePlan::MergePlan(const MergePlan& other) :
    lhs(other.lhs->duplicate()), rhs(other.rhs->duplicate()) { }


unique_ptr<JoinPlan> MergePlan::duplicate() {
    return make_unique<MergePlan>(*this);
}


void MergePlan::print() {
    cout << "MergePlan(";
    lhs->print();
    cout << ",";
    rhs->print();
    cout << ")";
}


double MergePlan::estimate_cost() {
    // TODO:
    return numeric_limits<double>::max() - 1;
    // return estimate_cost(*lhs, *rhs);
}


double MergePlan::estimate_cost(JoinPlan&, JoinPlan&) {
    return numeric_limits<double>::max() - 1; // TODO:
    // return lhs.estimate_cost() + rhs.estimate_cost();
}


double MergePlan::estimate_output_size() {
    // TODO:
    return lhs->estimate_output_size() * rhs->estimate_output_size();
}


bool MergePlan::cartesian_product_needed(JoinPlan& other) {
    for (auto& my_var : get_var_order()) {
        for (auto& other_var : other.get_var_order()) {
            if (my_var == other_var) {
                return false;
            }
        }
    }
    return true;
}


vector<VarId> MergePlan::get_var_order() {
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


void MergePlan::set_input_vars(std::vector<VarId>& input_var_order) {
    lhs->set_input_vars(input_var_order);
    auto left_var_order = lhs->get_var_order();
    rhs->set_input_vars(left_var_order);
}


unique_ptr<BindingIdIter> MergePlan::get_binding_id_iter() {
    // TODO: calcular variables de intersección
    std::vector<VarId> join_vars;
    return make_unique<MergeJoin>(lhs->get_binding_id_iter(), rhs->get_binding_id_iter(), move(join_vars));
}