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
    String,
    Var
};

class Value {
public:
    virtual std::unique_ptr<std::vector<char>> get_bytes() const = 0;
    virtual std::string to_string() = 0;
    virtual ValueType type() = 0;
};

#endif //BASE__VALUE_H_
