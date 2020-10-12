#ifndef BASE__OBJECT_TYPE_H_
#define BASE__OBJECT_TYPE_H_

#include <memory>
#include <string>

enum class ObjectType {
    identifiable_node,
    anonymous_node,
    edge,
    value_bool,
    value_float,
    value_int,
    value_string
};

class GraphObject {
public:
    virtual ~GraphObject() = default;
    virtual std::string to_string() const noexcept = 0;
    virtual ObjectType type()       const noexcept = 0;

    virtual bool operator==(const GraphObject& rhs) const noexcept = 0;
    virtual bool operator!=(const GraphObject& rhs) const noexcept = 0;
    virtual bool operator<=(const GraphObject& rhs) const noexcept = 0;
    virtual bool operator>=(const GraphObject& rhs) const noexcept = 0;
    virtual bool operator<(const GraphObject& rhs)  const noexcept = 0;
    virtual bool operator>(const GraphObject& rhs)  const noexcept = 0;
};

#endif // BASE__OBJECT_TYPE_H_
