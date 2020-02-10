#ifndef RELATIONAL_MODEL__BINDING_MATCH_H_
#define RELATIONAL_MODEL__BINDING_MATCH_H_

#include "base/binding/binding.h"
#include "base/var/var_id.h"

#include <map>

class BindingId;
class ObjectFile;

class BindingMatch : public Binding {

private:
    ObjectFile& object_file;
    std::map<std::string, VarId> var_pos; // TODO: maybe is better to use a vector?
    std::unique_ptr<BindingId> binding_id;
    std::map<std::string, std::shared_ptr<Value>> cache;

public:
    BindingMatch(ObjectFile& object_file, std::map<std::string, VarId>& var_pos,
        std::unique_ptr<BindingId> binding_id);
    ~BindingMatch() = default;

    void print() const;
    std::shared_ptr<Value> operator[](const std::string& var);
    std::shared_ptr<Value> try_extend(const std::string& var, const std::string& key);
};

#endif //RELATIONAL_MODEL__BINDING_MATCH_H_
