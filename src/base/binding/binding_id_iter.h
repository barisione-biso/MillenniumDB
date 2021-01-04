#ifndef BASE__BINDING_ID_ITER_H_
#define BASE__BINDING_ID_ITER_H_

#include "base/binding/binding_id.h"

// Abstract class
class BindingIdIter {
public:
    // BindingIdIter(std::size_t binding_size) :
    //     my_binding(binding_size) { }

    // BindingIdIter(const BindingIdIter& other) = delete;

    virtual ~BindingIdIter() = default;

    // parent_binding is the input and the BindingIdIter must write its results in there
    virtual void begin(BindingId& parent_binding, bool parent_has_next) = 0;
    virtual void reset() = 0;
    virtual bool next() = 0; // next returning false means there are not more bindings

    // prints execution statistics
    virtual void analyze(int indent = 0) const = 0;

// protected:
//     BindingId my_binding;
};

#endif // BASE__BINDING_ID_ITER_H_
