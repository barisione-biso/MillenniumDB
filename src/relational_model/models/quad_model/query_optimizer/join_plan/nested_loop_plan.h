#ifndef RELATIONAL_MODEL__NESTED_LOOP_PLAN_H_
#define RELATIONAL_MODEL__NESTED_LOOP_PLAN_H_

#include "base/graph/graph_object.h"
#include "relational_model/models/quad_model/query_optimizer/join_plan/join_plan.h"

class NestedLoopPlan : public JoinPlan {
public:
    NestedLoopPlan(const NestedLoopPlan& other);
    NestedLoopPlan(std::unique_ptr<JoinPlan> lhs, std::unique_ptr<JoinPlan> rhs);
    ~NestedLoopPlan() = default;

    double estimate_cost() override;
    double estimate_output_size() override;

    uint64_t get_vars() override;
    void set_input_vars(const uint64_t input_vars) override;

    std::unique_ptr<BindingIdIter> get_binding_id_iter() override;
    std::unique_ptr<JoinPlan> duplicate() override;

    void print(int indent, bool estimated_cost, std::vector<std::string>& var_names) override;
private:
    std::unique_ptr<JoinPlan> lhs;
    std::unique_ptr<JoinPlan> rhs;

    double output_size;
    double cost;
};

#endif // RELATIONAL_MODEL__NESTED_LOOP_PLAN_H_
