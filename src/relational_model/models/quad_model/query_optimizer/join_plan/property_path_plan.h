#ifndef RELATIONAL_MODEL__PROPERTY_PATH_PLAN_H_
#define RELATIONAL_MODEL__PROPERTY_PATH_PLAN_H_

#include "relational_model/models/quad_model/quad_model.h"
#include "relational_model/models/quad_model/query_optimizer/join_plan/join_plan.h"
#include "base/parser/logical_plan/op/property_paths/path_automaton.h"


class PropertyPathPlan : public JoinPlan {
public:
    PropertyPathPlan(const PropertyPathPlan& other);
    PropertyPathPlan(const QuadModel& model, VarId path_var, Id from, Id to, OpPath& path);
    ~PropertyPathPlan() = default;

    double estimate_cost() override;
    double estimate_output_size() override;

    uint64_t get_vars() override;
    void set_input_vars(const uint64_t input_vars) override;

    std::unique_ptr<BindingIdIter> get_binding_id_iter() override;

    std::unique_ptr<LeapfrogIter> get_leapfrog_iter(const std::set<VarId>&    /*assigned_vars*/,
                                                    const std::vector<VarId>& /*var_order*/,
                                                    uint_fast32_t             /*enumeration_level*/) override
                                                    { return nullptr; }

    std::unique_ptr<JoinPlan> duplicate() override;

    void print(int indent, bool estimated_cost, std::vector<std::string>& var_names) override;

    // Set transitions with TransitionId object (string to ObjectId)
    void set_automaton_transition_id(PathAutomaton& automaton);

private:
    const QuadModel& model;
    const VarId path_var;
    const Id from;
    const Id to;
    OpPath& path;

    bool from_assigned;
    bool to_assigned;
};

#endif // RELATIONAL_MODEL__PROPERTY_PATH_PLAN_H_
