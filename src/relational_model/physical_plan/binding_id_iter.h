#ifndef RELATIONAL_MODEL__BINDING_ID_ITER_H_
#define RELATIONAL_MODEL__BINDING_ID_ITER_H_

#include <memory>
#include <vector>

#include <base/var/var_id.h>
#include <relational_model/physical_plan/binding_id.h>

// Abstract class
class BindingIdIter {
public:
    virtual void init(BindingId& input) = 0;
    virtual void reset(BindingId& input) = 0;
    virtual BindingId* next() = 0; // next returning nullptr means there are not more bindings
};

#endif //RELATIONAL_MODEL__BINDING_ID_ITER_H_
