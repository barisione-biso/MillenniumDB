#ifndef BASE__VALUE_INT_H_
#define BASE__VALUE_INT_H_

#include <cstring>

#include "base/graph/value/value.h"

class ValueInt: public Value {
public:
    const int64_t value;

    ValueInt(int64_t value);
    ~ValueInt();

    std::string to_string() const noexcept override;
    ObjectType type()       const noexcept override;

    bool operator==(const GraphObject& rhs) const noexcept override;
    bool operator!=(const GraphObject& rhs) const noexcept override;
    bool operator<=(const GraphObject& rhs) const noexcept override;
    bool operator>=(const GraphObject& rhs) const noexcept override;
    bool operator<(const GraphObject& rhs)  const noexcept override;
    bool operator>(const GraphObject& rhs)  const noexcept override;
};

#endif // BASE__VALUE_INT_H_
