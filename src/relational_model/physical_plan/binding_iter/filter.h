#ifndef RELATIONAL_MODEL__FILTER_H_
#define RELATIONAL_MODEL__FILTER_H_

#include "base/binding/binding_iter.h"
#include "base/ids/var_id.h"
#include "relational_model/binding/binding_id_iter.h"

#include <map>
#include <memory>

class Condition;

class Filter : public BindingIter {

private:
    std::unique_ptr<BindingIter> iter;
    std::unique_ptr<Condition> condition;
    std::map<std::string, VarId> id_map;

public:
    Filter(std::unique_ptr<BindingIter> iter, std::unique_ptr<Condition> condition, std::map<std::string, VarId> id_map);
    ~Filter() = default;

    void begin();
    std::unique_ptr<Binding> next();
};

#endif //RELATIONAL_MODEL__FILTER_H_
