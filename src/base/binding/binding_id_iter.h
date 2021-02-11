#ifndef BASE__BINDING_ID_ITER_H_
#define BASE__BINDING_ID_ITER_H_

#include "base/binding/binding_id.h"

// Abstract class
class BindingIdIter {
public:

    virtual ~BindingIdIter() = default;

    // parent_binding is the input and the BindingIdIter must write its results in there
    virtual void begin(BindingId& parent_binding, bool parent_has_next) = 0;
    virtual void reset() = 0;
    virtual bool next() = 0; // next returning false means there are not more bindings
    virtual void assign_nulls() = 0;

    // prints execution statistics
    virtual void analyze(int indent = 0) const = 0;
};

#endif // BASE__BINDING_ID_ITER_H_
