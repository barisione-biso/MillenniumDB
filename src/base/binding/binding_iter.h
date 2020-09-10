#ifndef BASE__BINDING_ITER_H_
#define BASE__BINDING_ITER_H_

#include <memory>

#include "base/binding/binding.h"

// Abstract class
class BindingIter {
public:
    virtual ~BindingIter() = default;
    virtual void begin() = 0;
    virtual std::unique_ptr<Binding> next() = 0; // next returning nullptr means there are not more bindings

    // prints execution statistics
    virtual void analyze(int indent = 0) const = 0;
};

#endif // BASE__BINDING_ITER_H_
