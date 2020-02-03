#ifndef RELATIONAL_MODEL__BINDING_ROOT_H_
#define RELATIONAL_MODEL__BINDING_ROOT_H_

#include "base/binding/binding.h"

class BindingRoot : public Binding {

public:
    BindingRoot();
    ~BindingRoot() = default;

    void print() const;
    std::shared_ptr<Value> operator[](const std::string& var);
};

#endif //RELATIONAL_MODEL__BINDING_ROOT_H_
