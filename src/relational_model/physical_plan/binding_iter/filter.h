#ifndef RELATIONAL_MODEL__FILTER_H_
#define RELATIONAL_MODEL__FILTER_H_

#include "base/binding/binding_iter.h"
#include "relational_model/binding/binding_id_iter.h"

#include <memory>

class Condition;

class Filter : public BindingIter {

private:
    std::unique_ptr<BindingIter> iter;
    std::unique_ptr<Condition> condition;

public:
    Filter(std::unique_ptr<BindingIter> iter, std::unique_ptr<Condition> condition);
    ~Filter() = default;

    void begin();
    std::unique_ptr<Binding> next();
};

#endif //RELATIONAL_MODEL__FILTER_H_
