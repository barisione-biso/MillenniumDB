#ifndef RELATIONAL_MODEL__BINDING_FILTER_H_
#define RELATIONAL_MODEL__BINDING_FILTER_H_

#include <map>

#include "base/binding/binding.h"
#include "base/graph/graph_object.h"
#include "base/graph/graph_model.h"
#include "base/ids/var_id.h"

class BindingWhere : public Binding {
public:
    BindingWhere(const GraphModel& model,
                 Binding& child_binding,
                 VarId max_var_id_in_child,
                 std::map<VarId, std::pair<VarId, ObjectId>> property_map);
    ~BindingWhere() = default;

    std::ostream& print_to_ostream(std::ostream&) const override;

    void print_header(std::ostream&) const override { }

    GraphObject operator[](const VarId var_id) override;

    void clear_cache(); // needs to be called when the binding changes

private:
    const GraphModel& model;
    VarId max_var_id_in_child;
    Binding& child_binding;

    std::map<VarId, GraphObject> cache;

    // property_var_id -> (property_var_id, property_key_object_id)
    std::map<VarId, std::pair<VarId, ObjectId>> property_map;
};

#endif // RELATIONAL_MODEL__BINDING_FILTER_H_
