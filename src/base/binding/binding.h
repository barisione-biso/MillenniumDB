#ifndef BASE__BINDING_H_
#define BASE__BINDING_H_

#include <memory>
#include <string>

#include "base/graph/graph_object.h"
#include "base/ids/var_id.h"

// Abstract class
class Binding {
    virtual std::ostream& print_to_ostream(std::ostream&) const = 0;
public:
    virtual ~Binding() = default;

    friend std::ostream& operator<<(std::ostream& os, const Binding& b) {
        return b.print_to_ostream(os);
    }

    virtual GraphObject operator[](const VarId var_id) = 0;
};

#endif // BASE__BINDING_H_
