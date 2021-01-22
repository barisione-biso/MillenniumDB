#ifndef RELATIONAL_MODEL__TRANSITIVE_CLOSURE_PLAN_H_
#define RELATIONAL_MODEL__TRANSITIVE_CLOSURE_PLAN_H_

#include "relational_model/models/quad_model/quad_model.h"
#include "relational_model/models/quad_model/query_optimizer/join_plan/join_plan.h"

class TransitiveClosurePlan : public JoinPlan {
public:
    TransitiveClosurePlan(const TransitiveClosurePlan& other);
    TransitiveClosurePlan(QuadModel& model, Id from, Id to, ObjectId type);
    ~TransitiveClosurePlan() = default;

    double estimate_cost() override;
    double estimate_output_size() override;

    uint64_t get_vars() override;
    void set_input_vars(const uint64_t input_vars) override;

    std::unique_ptr<BindingIdIter> get_binding_id_iter(std::size_t binding_size) override;
    std::unique_ptr<JoinPlan> duplicate() override;

    void print(int indent, bool estimated_cost, std::vector<std::string>& var_names) override;

private:
    QuadModel& model;
    const Id from;
    const Id to;
    const ObjectId type;

    bool from_assigned;
    bool to_assigned;
};

#endif // RELATIONAL_MODEL__TRANSITIVE_CLOSURE_PLAN_H_
