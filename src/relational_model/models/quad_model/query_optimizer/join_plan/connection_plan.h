#ifndef RELATIONAL_MODEL__CONNECTION_PLAN_H_
#define RELATIONAL_MODEL__CONNECTION_PLAN_H_

#include <string>

#include "base/graph/graph_object.h"
#include "relational_model/execution/binding_id_iter/scan_ranges/scan_range.h"
#include "relational_model/models/quad_model/quad_model.h"
#include "relational_model/models/quad_model/query_optimizer/join_plan/join_plan.h"

class ConnectionPlan : public JoinPlan {
public:
    ConnectionPlan(const ConnectionPlan& other);
    ConnectionPlan(const QuadModel& model, Id from, Id to, Id type, VarId edge);
    ~ConnectionPlan() = default;

    double estimate_cost() override;
    double estimate_output_size() override;

    void set_input_vars(const uint64_t input_vars) override;
    uint64_t get_vars() override;

    std::unique_ptr<BindingIdIter> get_binding_id_iter(std::size_t binding_size) override;

    std::unique_ptr<LeapfrogIter> get_leapfrog_iter(const std::set<VarId>&    assigned_vars,
                                                    const std::vector<VarId>& var_order,
                                                    uint_fast32_t             enumeration_level) override;

    std::unique_ptr<JoinPlan> duplicate() override;

    void print(int indent, bool estimated_cost, std::vector<std::string>& var_names) override;

private:
    const QuadModel& model;

    Id from;
    Id to;
    Id type;
    VarId edge;

    bool from_assigned;
    bool to_assigned;
    bool type_assigned;
    bool edge_assigned;
};

#endif // RELATIONAL_MODEL__CONNECTION_PLAN_H_
