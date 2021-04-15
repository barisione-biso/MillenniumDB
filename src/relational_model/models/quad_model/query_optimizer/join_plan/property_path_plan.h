#ifndef RELATIONAL_MODEL__PROPERTY_PATH_PLAN_H_
#define RELATIONAL_MODEL__PROPERTY_PATH_PLAN_H_

#include "relational_model/models/quad_model/quad_model.h"
#include "relational_model/models/quad_model/query_optimizer/join_plan/join_plan.h"
#include "base/parser/logical_plan/op/path_automaton/path_automaton.h"


class PropertyPathPlan : public JoinPlan {
public:
    PropertyPathPlan(const PropertyPathPlan& other);
    PropertyPathPlan(QuadModel& model, Id from, Id to, OpPath& path);
    ~PropertyPathPlan() = default;

    double estimate_cost() override;
    double estimate_output_size() override;

    uint64_t get_vars() override;
    void set_input_vars(const uint64_t input_vars) override;

    std::unique_ptr<BindingIdIter> get_binding_id_iter(std::size_t binding_size) override;
    std::unique_ptr<JoinPlan> duplicate() override;

    void print(int indent, bool estimated_cost, std::vector<std::string>& var_names) override;

    // Set transitions with TransitionId object (string to ObjectId)
    void transform_automaton(PathAutomaton& automaton);

private:
    QuadModel& model;
    const Id from;
    const Id to;
    OpPath& path;

    bool from_assigned;
    bool to_assigned;
};

#endif // RELATIONAL_MODEL__PROPERTY_PATH_PLAN_H_
