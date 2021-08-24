#ifndef BASE__BINDING_ITER_H_
#define BASE__BINDING_ITER_H_

#include "base/binding/binding.h"

// Abstract class
class BindingIter {
public:
    virtual ~BindingIter() = default;

    // returns the position where all the results will be written
    virtual Binding& get_binding() noexcept = 0;

    // begin has to be called before calling next()
    virtual void begin() = 0;

    // returns true if there are more bindings and false otherwise
    virtual bool next() = 0;

    // prints execution statistics
    virtual void analyze(int indent = 0) const = 0;
};

#endif // BASE__BINDING_ITER_H_
