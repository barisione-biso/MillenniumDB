#ifndef RELATIONAL_MODEL__BINDING_ID_ITER_H_
#define RELATIONAL_MODEL__BINDING_ID_ITER_H_

#include <memory>
#include <vector>

#include <base/var/var_id.h>
#include <relational_model/physical_plan/binding_id.h>

// Abstract class
class BindingIdIter {
public:
    virtual void init(std::shared_ptr<BindingIdRange const> input) = 0;
    virtual void reset(std::shared_ptr<BindingIdRange const> input) = 0;
    virtual std::unique_ptr<BindingId const> next() = 0; // next returning nullptr means there are not more bindings
};

#endif //RELATIONAL_MODEL__BINDING_ID_ITER_H_
