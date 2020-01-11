#ifndef BASE__VALUE_H_
#define BASE__VALUE_H_

#include <memory>
#include <vector>
#include <string>

#include "base/var/var_id.h"
#include "base/graph/value/value.h"

class Value {
public:
    virtual std::unique_ptr<std::vector<char>> get_bytes() const = 0;
    virtual std::string to_string() = 0;
    virtual bool is_var() = 0;
    virtual VarId get_var() = 0;
};

#endif //BASE__VALUE_H_
