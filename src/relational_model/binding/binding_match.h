#ifndef RELATIONAL_MODEL__BINDING_MATCH_H_
#define RELATIONAL_MODEL__BINDING_MATCH_H_

#include "base/binding/binding.h"
#include "base/ids/var_id.h"

#include <map>

class BindingId;
class ObjectFile;

class BindingMatch : public Binding {

private:
    const std::map<std::string, VarId> var_pos;
    std::unique_ptr<BindingId> binding_id;
    std::map<std::string, std::shared_ptr<Value>> cache;

public:
    BindingMatch(const std::map<std::string, VarId>& var_pos,
        std::unique_ptr<BindingId> binding_id);
    ~BindingMatch() = default;

    void print() const;
    std::shared_ptr<Value> operator[](const std::string& var);
    std::shared_ptr<Value> try_extend(const std::string& var, const std::string& key);
};

#endif //RELATIONAL_MODEL__BINDING_MATCH_H_
