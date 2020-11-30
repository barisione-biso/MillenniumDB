#ifndef RELATIONAL_MODEL__BINDING_FILTER_H_
#define RELATIONAL_MODEL__BINDING_FILTER_H_

#include <map>

#include "base/binding/binding.h"
#include "base/graph/graph_object.h"
#include "base/graph/graph_model.h"
#include "base/ids/var_id.h"

class BindingWhere : public Binding {
public:
    BindingWhere(GraphModel& model, Binding& child_binding, VarId max_var_id_in_child,
                 std::map<VarId, std::pair<VarId, ObjectId>> property_map);
    ~BindingWhere() = default;

    std::string to_string() const override;

    std::shared_ptr<GraphObject> operator[](const VarId var_id) override;
    ObjectId get_id(const VarId var_id) override;


    void clear_cache(); // needs to be called when the binding changes

private:
    GraphModel& model;
    VarId max_var_id_in_child;
    Binding& child_binding;

    std::map<VarId, std::shared_ptr<GraphObject>> cache;

    // property_var_id -> (property_var_id, property_key_object_id)
    std::map<VarId, std::pair<VarId, ObjectId>> property_map;
};

#endif // RELATIONAL_MODEL__BINDING_FILTER_H_
