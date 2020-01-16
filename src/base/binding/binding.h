#ifndef BASE__BINDING_H_
#define BASE__BINDING_H_

#include <iostream>
#include <iterator>
#include <map>

#include "base/var/var_id.h"
#include "relational_model/graph/object_id.h"

class ObjectFile;

class Binding { // TODO: abstract class, implemented more efficiently inside Iter?
    private:
        std::map<std::string, std::string> dict;
    public:
        Binding();
        ~Binding() = default;
        int_fast32_t var_count();
        void add(std::string name, std::string value); // por ahora solo string, TODO: variant de valores?
        void print() const;
};


#endif //BASE__BINDING_H_
