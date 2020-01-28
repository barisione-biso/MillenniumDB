#ifndef BASE__VARIANT_VALUE_H_
#define BASE__VARIANT_VALUE_H_

#include <string>
#include <variant>

#include "base/var/var_id.h"

typedef std::variant<int, float, bool, std::string, VarId> VariantValue;

#endif //BASE__VARIANT_VALUE_H_
