#ifndef RELATIONAL_MODEL__BINDING_ORDER_BY_H_
#define RELATIONAL_MODEL__BINDING_ORDER_BY_H_

#include <memory>
#include <string>
#include <vector>

#include "base/binding/binding.h"
#include "base/binding/binding_iter.h"
#include "base/binding/binding_id.h"
#include "base/graph/graph_model.h"

class BindingOrderBy : public Binding {
public:
    BindingOrderBy(GraphModel& model, std::vector<std::pair<std::string, VarId>> order_vars, Binding& child_binding, size_t binding_size);
    ~BindingOrderBy();

    std::string to_string() const override;

    std::shared_ptr<GraphObject> operator[](const VarId var_id) override;
    ObjectId get_id(const VarId var_id) override;
    void update_binding_object(std::vector<uint64_t> obj_ids);


private:
    GraphModel& model;
    size_t binding_size;
    std::vector<std::pair<std::string, VarId>> order_vars;
    ObjectId* binding_id;
    Binding& child_binding;
};

#endif // RELATIONAL_MODEL__BINDING_ORDER_BY_H_
