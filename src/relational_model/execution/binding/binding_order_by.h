#ifndef RELATIONAL_MODEL__BINDING_ORDER_BY_H_
#define RELATIONAL_MODEL__BINDING_ORDER_BY_H_

#include "base/binding/binding.h"
#include "base/binding/binding_iter.h"
#include "base/binding/binding_id.h"

#include <memory>
#include <string>
#include <vector>

class BindingOrderBy : public Binding {
public:
    BindingOrderBy(std::vector<std::pair<std::string, VarId>> order_vars, Binding& child_binding);
    ~BindingOrderBy() = default;

    std::string to_string() const override;

    std::shared_ptr<GraphObject> operator[](const VarId var_id) override;
    ObjectId get_id(const VarId var_id) override;


private:
    std::vector<std::pair<std::string, VarId>> order_vars;
    Binding& child_binding;
};

#endif // RELATIONAL_MODEL__BINDING_ORDER_BY_H_
