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
    BindingOrderBy(const GraphModel& model, size_t binding_size);
    ~BindingOrderBy() = default;

    std::ostream& print_to_ostream(std::ostream&) const override;
    GraphObject operator[](const VarId var_id) override;
    void update_binding_object(std::vector<GraphObject> graph_object);

private:
    const GraphModel& model;
    size_t binding_size;
    std::vector<GraphObject> objects_vector;
};

#endif // RELATIONAL_MODEL__BINDING_ORDER_BY_H_
