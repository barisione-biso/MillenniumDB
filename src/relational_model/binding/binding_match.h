#ifndef RELATIONAL_MODEL__BINDING_MATCH_H_
#define RELATIONAL_MODEL__BINDING_MATCH_H_

#include "base/binding/binding.h"
#include "base/ids/var_id.h"

#include <map>

class BindingId;

class BindingMatch : public Binding {

private:
    const std::map<std::string, VarId> var_pos;
    std::unique_ptr<BindingId> binding_id;
    std::map<std::string, std::shared_ptr<GraphObject>> cache;

public:
    BindingMatch(const std::map<std::string, VarId>& var_pos,
        std::unique_ptr<BindingId> binding_id);
    ~BindingMatch();

    std::string to_string() const override;

    std::shared_ptr<GraphObject> operator[](const std::string& var) override;
    std::shared_ptr<GraphObject> get(const std::string& var, const std::string& key) override;
};

#endif // RELATIONAL_MODEL__BINDING_MATCH_H_
