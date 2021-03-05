#ifndef RELATIONAL_MODEL__BINDING_DISTINCT_H_
#define RELATIONAL_MODEL__BINDING_DISTINCT_H_

#include <vector>

#include "base/binding/binding.h"
#include "base/graph/graph_model.h"

class BindingDistinct : public Binding {
public:
    BindingDistinct(GraphModel& model);
    ~BindingDistinct() = default;

    std::ostream& print_to_ostream(std::ostream&) const override;

    GraphObject operator[](const VarId var_id) override;

private:
    GraphModel& model;
    std::vector<GraphObject> objects_vector;

};

#endif // RELATIONAL_MODEL__BINDING_DISTINCT_H_
