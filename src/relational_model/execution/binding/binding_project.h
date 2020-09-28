#ifndef RELATIONAL_MODEL__BINDING_PROJECT_H_
#define RELATIONAL_MODEL__BINDING_PROJECT_H_

#include "base/binding/binding.h"
#include "base/binding/binding_iter.h"

#include <memory>
#include <string>
#include <vector>

class BindingProject : public Binding {

private:
    std::vector<std::string>& projection_vars;
    std::unique_ptr<Binding> current_binding;

public:
    BindingProject(std::vector<std::string>& projection_vars, std::unique_ptr<Binding> current_binding);
    ~BindingProject() = default;

    std::string to_string() const override;

    std::shared_ptr<GraphObject> operator[](const std::string& var) override;
    std::shared_ptr<GraphObject> get(const std::string& var, const std::string& key) override;
};

#endif // RELATIONAL_MODEL__BINDING_PROJECT_H_
