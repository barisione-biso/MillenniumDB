#ifndef RELATIONAL_MODEL__BINDING_GROUP_BY_H_
#define RELATIONAL_MODEL__BINDING_GROUP_BY_H_

#include <memory>
#include <string>
#include <vector>

#include "base/binding/binding.h"
#include "base/graph/graph_model.h"


class BindingId;

class BindingGroupBy : public Binding {
public:
    BindingGroupBy(GraphModel& model, std::vector<std::pair<std::string, VarId>> _group_vars, Binding& child_binding, size_t binding_size);
    ~BindingGroupBy();
    std::vector<std::pair<std::string, VarId>> group_vars;
    std::ostream& print_to_ostream(std::ostream&) const override;
    GraphObject operator[](const VarId var_id) override;
    void begin(BindingId&);


private:
    GraphModel& model;
    size_t binding_size;
    std::vector<GraphObject> objects_vector;
    Binding& child_binding;
};

#endif // RELATIONAL_MODEL__BINDING_GROUP_BY_H_
