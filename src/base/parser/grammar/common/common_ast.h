#ifndef BASE__COMMON_AST_H_
#define BASE__COMMON_AST_H_

#include <string>

#include <boost/variant.hpp>
#include <boost/optional.hpp>
#include <boost/fusion/include/io.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>

#include "base/ids/graph_id.h"
#include "base/ids/var_id.h"

namespace ast {
    namespace x3 = boost::spirit::x3;
    using Value = boost::variant<std::string, int64_t, float, bool>;

    struct Var {
        std::string name;
        operator std::string() { return name; }
    };

    struct Property {
        std::string key;
        Value value;
    };

    enum class EdgeDirection {
        right,
        left
    };
}

#endif // BASE__COMMON_AST_H_
