#ifndef BASE__VALUE_BOOL_H
#define BASE__VALUE_BOOL_H

#include "base/graph/element_type.h"
#include "base/graph/value/value.h"
#include "base/ids/object_id.h"

class ValueElement: public Value {

public:
    ObjectId value;
    ElementType element_type;

    ValueElement(ElementType element_type, ObjectId value)
        : element_type(element_type), value(value) { }
    ~ValueElement() = default;

    std::unique_ptr<std::vector<char>> get_bytes() const {
        // return std::make_unique<std::vector<char>>(1, (char)value);
        return nullptr; // TODO: pensar que debería devolver
    }

    std::string to_string() const {
        return std::to_string(value);
    }

    ValueType type() const {
        return ValueType::Element;
    }

    bool operator==(const Value& rhs) const {
        if (rhs.type() == ValueType::Element) {
            const auto& casted_rhs = dynamic_cast<const ValueElement&>(rhs);
            return this->value == casted_rhs.value && this->element_type == casted_rhs.element_type;
        }
        return false;
    }
};


#endif // BASE__VALUE_BOOL_H