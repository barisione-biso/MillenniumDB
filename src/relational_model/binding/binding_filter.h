#ifndef RELATIONAL_MODEL__BINDING_FILTER_H_
#define RELATIONAL_MODEL__BINDING_FILTER_H_

#include "base/binding/binding.h"
#include "base/ids/var_id.h"

#include <map>

class BindingId;

class BindingFilter : public Binding {

private:
    const std::map<std::string, VarId> var_pos;
    std::map<std::string, std::shared_ptr<Value>> cache;

public:
    BindingFilter(std::unique_ptr<BindingId> binding, const std::map<std::string, VarId>& var_pos);
    ~BindingFilter() = default;

    void print() const;
    std::shared_ptr<Value> operator[](const std::string& var);
    std::shared_ptr<Value> get_value(const std::string& var, const std::string& key);
};

#endif //RELATIONAL_MODEL__BINDING_FILTER_H_
