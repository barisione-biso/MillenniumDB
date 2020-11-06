#ifndef RELATIONAL_MODEL__LABEL_PLAN_H_
#define RELATIONAL_MODEL__LABEL_PLAN_H_

#include <variant>


#include "base/graph/graph_object.h"
#include "base/ids/object_id.h"
#include "relational_model/models/quad_model/quad_model.h"
#include "relational_model/models/quad_model/query_optimizer/join_plan/join_plan.h"
#include "relational_model/execution/binding_id_iter/scan_ranges/scan_range.h"

class LabelPlan : public JoinPlan {
public:
    LabelPlan(const LabelPlan& other);
    LabelPlan(QuadModel& model, Id node, Id label);
    ~LabelPlan() = default;

    double estimate_cost() override;
    double estimate_output_size() override;

    void set_input_vars(const uint64_t input_vars) override;
    uint64_t get_vars() override;

    std::unique_ptr<BindingIdIter> get_binding_id_iter(std::size_t binding_size) override;
    std::unique_ptr<JoinPlan> duplicate() override;

    void print(int indent, bool estimated_cost, std::vector<std::string>& var_names) override;

private:
    QuadModel& model;

    Id node;
    Id label;

    bool node_assigned;
    bool label_assigned;
};

#endif // RELATIONAL_MODEL__LABEL_PLAN_H_
