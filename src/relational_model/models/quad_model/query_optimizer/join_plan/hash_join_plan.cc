#include "hash_join_plan.h"
#include "relational_model/execution/binding_id_iter/hash_join/hash_join.h"
#include "relational_model/execution/binding_id_iter/hash_join/hash_join_in_memory.h"

using namespace std;

HashJoinPlan::HashJoinPlan(unique_ptr<JoinPlan> _lhs, unique_ptr<JoinPlan> _rhs) :
    lhs(move(_lhs)), rhs(move(_rhs))
{
    // TODO:
    output_size = lhs->estimate_output_size() * rhs->estimate_output_size();
    cost = lhs->estimate_cost() + rhs->estimate_cost();
}


HashJoinPlan::HashJoinPlan(const HashJoinPlan& other) :
    lhs         (other.lhs->duplicate()),
    rhs         (other.rhs->duplicate()),
    output_size (other.output_size),
    cost        (other.cost) { }


unique_ptr<JoinPlan> HashJoinPlan::duplicate() {
    return make_unique<HashJoinPlan>(*this);
}


void HashJoinPlan::print(int indent, bool estimated_cost, std::vector<std::string>& var_names) {
    for (int i = 0; i < indent; ++i) {
        cout << ' ';
    }
    cout << "HashJoin(\n";
    lhs->print(indent + 2, estimated_cost, var_names);
    cout << ",\n";
    rhs->print(indent + 2, estimated_cost, var_names);
    cout << "\n";
    for (int i = 0; i < indent; ++i) {
        cout << ' ';
    }
    cout << ")";

    // lhs->print(indent, estimated_cost, var_names);
    // cout << ",\n";
    // rhs->print(indent, estimated_cost, var_names);
}


double HashJoinPlan::estimate_cost() {
    // auto lhs_output_size = lhs->estimate_output_size();
    // if (lhs_output_size > 1) {
    //     return lhs->estimate_cost() + ((lhs->estimate_output_size()) * rhs->estimate_cost());
    // } else {
    //     return lhs->estimate_cost() + (rhs->estimate_cost());
    // }

    return cost;
}


double HashJoinPlan::estimate_output_size() {
    // return lhs->estimate_output_size() * rhs->estimate_output_size();
    return output_size;
}


uint64_t HashJoinPlan::get_vars() {
    return lhs->get_vars() | rhs->get_vars();
}


void HashJoinPlan::set_input_vars(uint64_t /*input_var_order*/) {
    // lhs->set_input_vars(input_var_order);
    // auto left_var_order = lhs->get_var_order();
    // rhs->set_input_vars(left_var_order);
    throw std::logic_error("HashJoin only works for left deep plans.");
}


unique_ptr<BindingIdIter> HashJoinPlan::get_binding_id_iter(std::size_t binding_size) {
    auto common_vars_bitmap = lhs->get_vars() & rhs->get_vars();
    auto not_commons = ~common_vars_bitmap;
    auto left_vars_bitmap = lhs->get_vars() & not_commons;
    auto right_vars_bitmap = rhs->get_vars() & not_commons;

    std::vector<VarId> common_vars;
    std::vector<VarId> left_vars;
    std::vector<VarId> right_vars;
    for (uint_fast32_t position = 0; position < binding_size; position++) {
        auto current_mask = 1 << position; // from right to left
        if ((common_vars_bitmap & current_mask) > 0) {
            common_vars.push_back(VarId(position));
        }
        else if ((left_vars_bitmap & current_mask) > 0) {
            left_vars.push_back(VarId(position));
        }
        else if ((right_vars_bitmap & current_mask) > 0) {
            right_vars.push_back(VarId(position));
        }
    }

    return make_unique<HashJoin>( //<HashJoinInMemory>
        lhs->get_binding_id_iter(binding_size),
        rhs->get_binding_id_iter(binding_size),
        move(left_vars),
        move(common_vars),
        move(right_vars)
    );
}
