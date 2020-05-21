#include "match.h"

#include <cassert>
#include <limits>

#include "storage/index/object_file/object_file.h"
#include "relational_model/binding/binding_match.h"
#include "relational_model/query_optimizer/join_plan/join_plan.h"
#include "relational_model/query_optimizer/join_plan/nested_loop_plan.h"
#include "relational_model/query_optimizer/join_plan/merge_plan.h"

using namespace std;

Match::Match(vector<std::unique_ptr<JoinPlan>> base_plans, std::map<std::string, VarId> var_pos)
    : base_plans(move(base_plans)), var_pos(move(var_pos))
{
    root = get_greedy_join_plan();
}


void Match::begin() {
    binding_id = make_unique<BindingId>(var_pos.size());
    root->begin(*binding_id);
}


unique_ptr<Binding> Match::next() {
    auto binding_id_ptr = root->next();
    if (binding_id_ptr != nullptr) {
        auto binding_id_copy = make_unique<BindingId>(*binding_id_ptr);
        return make_unique<BindingMatch>(var_pos, move(binding_id_copy));
    } else {
        return nullptr;
    }
}


unique_ptr<BindingIdIter> Match::get_greedy_join_plan() {
    auto base_plans_size = base_plans.size();

    assert(base_plans_size > 0
        && "base_plans size in Match must be greater than 0");

    // choose the first scan
    int best_index = 0;
    double best_cost = std::numeric_limits<double>::max();
    for (size_t j = 0; j < base_plans_size; j++) {
        auto current_element_cost = base_plans[j]->estimate_cost();
        if (current_element_cost < best_cost) {
            best_cost = current_element_cost;
            best_index = j;
        }
    }
    auto root_plan = move(base_plans[best_index]);

    // choose the next scan and make a IndexNestedLoppJoin or MergeJoin
    for (size_t i = 1; i < base_plans_size; i++) {
        best_index = 0;
        best_cost = std::numeric_limits<double>::max();
        unique_ptr<JoinPlan> best_step_plan = nullptr;

        for (size_t j = 0; j < base_plans_size; j++) {
            if (base_plans[j] != nullptr
                && !base_plans[j]->cartesian_product_needed(*root_plan) )
            {
                auto nested_loop_cost = NestedLoopPlan::estimate_cost(*root_plan, *base_plans[j]);
                auto merge_cost = MergePlan::estimate_cost(*root_plan, *base_plans[j]);

                if (nested_loop_cost <= merge_cost) {
                    if (nested_loop_cost < best_cost) {
                        best_cost = nested_loop_cost;
                        best_index = j;
                        best_step_plan = make_unique<NestedLoopPlan>(root_plan->duplicate(), base_plans[j]->duplicate());
                    }
                } else { // merge_cost < nested_loop_cost
                    if (merge_cost < best_cost) {
                        best_cost = merge_cost;
                        best_index = j;
                        best_step_plan = make_unique<MergePlan>(root_plan->duplicate(), base_plans[j]->duplicate());
                    }
                }
            }
        }

        // All elements would form a cross product, iterate again, allowing cross products
        if (best_cost == std::numeric_limits<double>::max()) {
            best_index = 0;

            for (size_t j = 0; j < base_plans_size; j++) {
                if (base_plans[j] == nullptr) {
                    continue;
                }
                auto nested_loop_cost = NestedLoopPlan::estimate_cost(*root_plan, *base_plans[j]);
                auto merge_cost = MergePlan::estimate_cost(*root_plan, *base_plans[j]);

                if (nested_loop_cost <= merge_cost) {
                    if (nested_loop_cost < best_cost) {
                        best_cost = nested_loop_cost;
                        best_index = j;
                        best_step_plan = make_unique<NestedLoopPlan>(root_plan->duplicate(), base_plans[j]->duplicate());
                    }
                } else { // merge_cost < nested_loop_cost
                    if (merge_cost < best_cost) {
                        best_cost = merge_cost;
                        best_index = j;
                        best_step_plan = make_unique<MergePlan>(root_plan->duplicate(), base_plans[j]->duplicate());
                    }
                }
            }
        }
        base_plans[best_index] = nullptr;
        root_plan = move(best_step_plan);
    }
    root_plan->print();
    cout << "\n";
    return root_plan->get_binding_id_iter();
}
