#ifndef RELATIONAL_MODEL__BINDING_ID_ITER_H_
#define RELATIONAL_MODEL__BINDING_ID_ITER_H_

#include "relational_model/execution/binding/binding_id.h"

// Abstract class
class BindingIdIter {
public:
    virtual ~BindingIdIter() = default;
    virtual void begin(BindingId& input) = 0;
    virtual void reset(BindingId& input) = 0;
    virtual BindingId* next() = 0; // next returning nullptr means there are not more bindings

    // prints execution statistics
    virtual void analyze(int indent = 0) const = 0;
};

#endif // RELATIONAL_MODEL__BINDING_ID_ITER_H_
