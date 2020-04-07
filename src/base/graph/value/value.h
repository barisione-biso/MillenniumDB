#ifndef BASE__VALUE_H_
#define BASE__VALUE_H_

#include <memory>
#include <string>
#include <vector>

#include "base/graph/graph_object.h"

class Value : public GraphObject {
public:
    virtual ~Value() { };
    virtual std::unique_ptr<std::vector<unsigned char>> get_bytes() const = 0;
};


#include "base/graph/value/value_bool.h"
#include "base/graph/value/value_float.h"
#include "base/graph/value/value_int.h"
#include "base/graph/value/value_string.h"

#endif //BASE__VALUE_H_
