#ifndef RELATIONAL_MODEL__BINDING_FILTER_H_
#define RELATIONAL_MODEL__BINDING_FILTER_H_

#include "base/binding/binding.h"
#include "base/graph/graph_object.h"
#include "base/ids/graph_id.h"
#include "base/ids/var_id.h"

#include <set>
#include <map>

class BindingFilter : public Binding {

private:
    Binding& binding;
    std::map<std::string, std::shared_ptr<GraphObject>> cache;

public:
    BindingFilter(Binding& binding);
    ~BindingFilter() = default;

    std::string to_string() const override;

    std::shared_ptr<GraphObject> operator[](const std::string& var) override;
    std::shared_ptr<GraphObject> get(const std::string& var, const std::string& key) override;
};

#endif // RELATIONAL_MODEL__BINDING_FILTER_H_
