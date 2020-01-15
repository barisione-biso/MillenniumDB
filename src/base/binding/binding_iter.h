#ifndef BASE__BINDING_ITER_H_
#define BASE__BINDING_ITER_H_

#include <memory>
#include <vector>

#include <base/var/var_id.h>
#include <base/binding/binding.h>

// Abstract class
class BindingIter {
public:
    virtual std::unique_ptr<Binding> next() = 0; // next returning nullptr means there are not more bindings
};

#endif //BASE__BINDING_ITER_H_
