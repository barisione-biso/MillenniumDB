#ifndef RELATIONAL_MODEL__BINDING_ID_ITER_H_
#define RELATIONAL_MODEL__BINDING_ID_ITER_H_

#include "relational_model/execution/binding/binding_id.h"

// Abstract class
class BindingIdIter {
public:
    virtual ~BindingIdIter() = default;
    virtual BindingId* begin(BindingId& input) = 0;
    virtual void reset() = 0;
    virtual bool next() = 0; // next returning false means there are not more bindings

    // prints execution statistics
    virtual void analyze(int indent = 0) const = 0;

    // TODO: añadir my_binding aca?
};

#endif // RELATIONAL_MODEL__BINDING_ID_ITER_H_
