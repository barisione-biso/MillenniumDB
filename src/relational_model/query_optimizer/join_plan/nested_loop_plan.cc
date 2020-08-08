#include "nested_loop_plan.h"

#include "relational_model/execution/binding_id_iter/index_nested_loop_join.h"

using namespace std;

NestedLoopPlan::NestedLoopPlan(unique_ptr<JoinPlan> _lhs, unique_ptr<JoinPlan> _rhs) :
    lhs(move(_lhs)), rhs(move(_rhs))
{
    auto var_order = lhs->get_var_order();
    rhs->set_input_vars(var_order);

    auto lhs_output_size = lhs->estimate_output_size();
    output_size = lhs_output_size * rhs->estimate_output_size();

    if (lhs_output_size > 1) {
        cost = lhs->estimate_cost() + (lhs_output_size * rhs->estimate_cost());
    } else {
        cost = lhs->estimate_cost() + (rhs->estimate_cost());
    }
}


NestedLoopPlan::NestedLoopPlan(const NestedLoopPlan& other) :
    lhs         (other.lhs->duplicate()),
    rhs         (other.rhs->duplicate()),
    output_size (other.output_size),
    cost        (other.cost) { }


unique_ptr<JoinPlan> NestedLoopPlan::duplicate() {
    return make_unique<NestedLoopPlan>(*this);
}


void NestedLoopPlan::print(int indent, bool estimated_cost, std::vector<std::string>& var_names) {
    // for (int i = 0; i < indent; ++i) {
    //     cout << ' ';
    // }
    // cout << "IndexNestedLoopJoin(\n";
    // lhs->print(indent + 2, var_names);
    // cout << ",\n";
    // rhs->print(indent + 2, var_names);
    // cout << "\n";
    // for (int i = 0; i < indent; ++i) {
    //     cout << ' ';
    // }
    // cout << ")";

    lhs->print(indent, estimated_cost, var_names);
    cout << ",\n";
    rhs->print(indent, estimated_cost, var_names);
}


double NestedLoopPlan::estimate_cost() {
    // auto lhs_output_size = lhs->estimate_output_size();
    // if (lhs_output_size > 1) {
    //     return lhs->estimate_cost() + ((lhs->estimate_output_size()) * rhs->estimate_cost());
    // } else {
    //     return lhs->estimate_cost() + (rhs->estimate_cost());
    // }

    return cost;
}


double NestedLoopPlan::estimate_output_size() {
    // return lhs->estimate_output_size() * rhs->estimate_output_size();
    return output_size;
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


void NestedLoopPlan::set_input_vars(std::vector<VarId>& /*input_var_order*/) {
    // lhs->set_input_vars(input_var_order);
    // auto left_var_order = lhs->get_var_order();
    // rhs->set_input_vars(left_var_order);
    throw std::logic_error("NestedLoop only works for left deep plans.");
}


unique_ptr<BindingIdIter> NestedLoopPlan::get_binding_id_iter() {
    return make_unique<IndexNestedLoopJoin>(lhs->get_binding_id_iter(), rhs->get_binding_id_iter());
}
