#ifndef RELATIONAL_MODEL__BINDING_GROUP_BY_H_
#define RELATIONAL_MODEL__BINDING_GROUP_BY_H_

#include <memory>
#include <string>
#include <vector>

#include "base/binding/binding.h"
#include "base/graph/graph_model.h"
#include "base/parser/logical_plan/op/visitors/op_visitor.h"
#include "base/parser/logical_plan/var.h"

class BindingId;

class BindingGroupBy : public Binding {
public:
    BindingGroupBy(GraphModel& model, std::vector<std::pair<Var, VarId>> group_vars, Binding& child_binding, size_t binding_size);
    ~BindingGroupBy() = default;

    std::ostream& print_to_ostream(std::ostream&) const override;
    GraphObject operator[](const VarId var_id) override;
    void update_binding(std::vector<GraphObject>);

private:
    std::vector<std::pair<Var, VarId>> group_vars;
    GraphModel& model;
    size_t binding_size;
    Binding& child_binding;
    std::vector<GraphObject> current_objects;
};

#endif // RELATIONAL_MODEL__BINDING_GROUP_BY_H_
