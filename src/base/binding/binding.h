#ifndef BASE__BINDING_H_
#define BASE__BINDING_H_

#include <memory>
#include <string>

#include "base/graph/graph_object.h"
#include "base/ids/var_id.h"

// Abstract class
class Binding {
public:
    virtual ~Binding() = default;

    virtual std::string to_string() const = 0;

    virtual std::shared_ptr<GraphObject> operator[](const VarId var_id) = 0;
    virtual ObjectId get_id(const VarId var_id) = 0;

};

#endif // BASE__BINDING_H_
