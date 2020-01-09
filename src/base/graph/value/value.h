#ifndef BASE__VALUE_H_
#define BASE__VALUE_H_

#include <memory>
#include <vector>
#include <string>


class  Value {
public:
    virtual std::unique_ptr<std::vector<char>> get_bytes() const = 0;
    virtual std::string to_string() = 0;
};


#include "base/graph/value/value_bool.h"
#include "base/graph/value/value_float.h"
#include "base/graph/value/value_int.h"
#include "base/graph/value/value_string.h"

#endif //BASE__VALUE_H_
