#ifndef RELATIONAL_MODEL__PROPERTY_PLAN_H_
#define RELATIONAL_MODEL__PROPERTY_PLAN_H_

#include <variant>

#include "base/graph/graph_object.h"
#include "base/ids/object_id.h"
#include "relational_model/models/quad_model/quad_model.h"
#include "relational_model/models/quad_model/query_optimizer/join_plan/join_plan.h"
#include "relational_model/execution/binding_id_iter/scan_ranges/scan_range.h"

class PropertyPlan : public JoinPlan {
public:
    PropertyPlan(const PropertyPlan& other);
    PropertyPlan(QuadModel& model, Id object, Id key, Id value);

    ~PropertyPlan() = default;

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
    QuadModel& model;

    Id object;
    Id key;
    Id value;

    bool object_assigned;
    bool key_assigned;
    bool value_assigned;
};

#endif // RELATIONAL_MODEL__PROPERTY_PLAN_H_
