#ifndef RELATIONAL_MODEL__BINDING_ORDER_BY_H_
#define RELATIONAL_MODEL__BINDING_ORDER_BY_H_

#include <memory>
#include <string>
#include <vector>

#include "base/binding/binding.h"
#include "base/graph/graph_model.h"

class BindingId;

class BindingOrderBy : public Binding {
public:
    BindingOrderBy(GraphModel& model, std::vector<std::pair<std::string, VarId>> order_vars, Binding& child_binding, size_t binding_size);
    ~BindingOrderBy();
    std::vector<std::pair<std::string, VarId>> order_vars;
    std::ostream& print_to_ostream(std::ostream&) const override;
    GraphObject operator[](const VarId var_id) override;
    void finish_read_of_child();
    void begin(BindingId&);
    void update_binding_object(std::vector<GraphObject> graph_object);


private:
    GraphModel& model;
    size_t binding_size;
    std::vector<GraphObject> objects_vector;
    bool consume_from_child = true;
    Binding& child_binding;
};

#endif // RELATIONAL_MODEL__BINDING_ORDER_BY_H_
