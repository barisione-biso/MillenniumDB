#ifndef RELATIONAL_MODEL__BINDING_PROJECT_H_
#define RELATIONAL_MODEL__BINDING_PROJECT_H_

#include "base/binding/binding.h"
#include "base/binding/binding_iter.h"
#include "base/binding/binding_id.h"
#include "base/parser/logical_plan/var.h"

#include <memory>
#include <string>
#include <vector>

class BindingSelect : public Binding {
public:
    BindingSelect(std::vector<std::pair<Var, VarId>> projection_vars, Binding& child_binding);
    ~BindingSelect() = default;

    std::ostream& print_to_ostream(std::ostream&) const override;

    GraphObject operator[](const VarId var_id) override;

private:
    std::vector<std::pair<Var, VarId>> projection_vars;
    Binding& child_binding;
};

#endif // RELATIONAL_MODEL__BINDING_PROJECT_H_
