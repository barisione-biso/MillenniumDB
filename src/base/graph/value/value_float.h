#ifndef BASE__VALUE_FLOAT_H_
#define BASE__VALUE_FLOAT_H_

#include <cstring>

#include "base/graph/value/value.h"

class ValueFloat: public Value {
public:
    const float value;

    ValueFloat(float value);
    ~ValueFloat();

    std::string to_string() const noexcept override;
    ObjectType type()       const noexcept override;

    bool operator==(const GraphObject& rhs) const noexcept override;
    bool operator!=(const GraphObject& rhs) const noexcept override;
    bool operator<=(const GraphObject& rhs) const noexcept override;
    bool operator>=(const GraphObject& rhs) const noexcept override;
    bool operator<(const GraphObject& rhs)  const noexcept override;
    bool operator>(const GraphObject& rhs)  const noexcept override;
};

#endif // BASE__VALUE_FLOAT_H_
