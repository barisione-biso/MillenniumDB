#ifndef BASE__VALUE_H_
#define BASE__VALUE_H_

#include "base/var/var_id.h"
#include "base/graph/value/value.h"

#include <memory>
#include <string>
#include <vector>

enum class ValueType {
    Bool,
    Float,
    Int,
    String
};

class Value {
public:
    virtual std::unique_ptr<std::vector<char>> get_bytes() const = 0;
    virtual std::string to_string() const = 0;
    virtual ValueType type() const = 0;

    virtual bool operator==(const Value& rhs) const = 0;
};

#endif //BASE__VALUE_H_
