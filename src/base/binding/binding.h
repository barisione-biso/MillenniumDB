#ifndef BASE__BINDING_H_
#define BASE__BINDING_H_

#include <memory>
#include <string>

class Value;

// Abstract class
class Binding {

public:
    virtual void print() const = 0;
    virtual std::shared_ptr<Value> operator[](const std::string& var) = 0;
    virtual std::shared_ptr<Value> get(const std::string& var, const std::string& key) = 0;
    virtual std::shared_ptr<Value> try_extend(const std::string& var, const std::string& key) = 0;
};


#endif //BASE__BINDING_H_
