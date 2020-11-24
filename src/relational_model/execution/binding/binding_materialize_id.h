#ifndef RELATIONAL_MODEL__BINDING_MATERIALIZE_ID_H_
#define RELATIONAL_MODEL__BINDING_MATERIALIZE_ID_H_

#include <map>

#include "base/binding/binding.h"
#include "base/graph/graph_model.h"
#include "base/ids/var_id.h"

class BindingId;

class BindingMaterializeId : public Binding {
public:
    BindingMaterializeId(GraphModel& model, std::size_t binding_size, BindingId& binding_id);
    ~BindingMaterializeId();

    std::string to_string() const override;

    std::shared_ptr<GraphObject> operator[](const VarId var_id) override;

    void begin(BindingId&);

private:
    GraphModel& model;
    std::size_t binding_size;
    BindingId& binding_id;
};

#endif // RELATIONAL_MODEL__BINDING_MATERIALIZE_ID_H_
