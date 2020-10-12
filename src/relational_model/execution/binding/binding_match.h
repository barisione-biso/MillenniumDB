#ifndef RELATIONAL_MODEL__BINDING_MATCH_H_
#define RELATIONAL_MODEL__BINDING_MATCH_H_

#include <map>

#include "base/binding/binding.h"
#include "base/ids/var_id.h"
#include "relational_model/models/graph_model.h"

class BindingId;

class BindingMatch : public Binding {
public:
    BindingMatch(GraphModel& model, const std::map<std::string, VarId>& var_pos, BindingId* binding_id);
    ~BindingMatch();

    std::string to_string() const override;

    std::shared_ptr<GraphObject> operator[](const std::string& var) override;
    std::shared_ptr<GraphObject> get(const std::string& var, const std::string& key) override;

private:
    GraphModel& model;
    const std::map<std::string, VarId> var_pos;
    BindingId* binding_id;
    // std::map<std::string, std::shared_ptr<GraphObject>> cache;
};

#endif // RELATIONAL_MODEL__BINDING_MATCH_H_
