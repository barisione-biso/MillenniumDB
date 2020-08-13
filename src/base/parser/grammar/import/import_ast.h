#ifndef BASE__IMPORT_AST_H_
#define BASE__IMPORT_AST_H_

#include <iostream>
#include <string>

#include <boost/variant.hpp>
#include <boost/optional.hpp>
#include <boost/fusion/include/io.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>

#include "base/parser/grammar/common/common_ast.h"

namespace import_ast {
    namespace x3 = boost::spirit::x3;
    using namespace ast;

    struct Node {
        uint64_t id;
        std::vector<std::string> labels;
        std::vector<Property> properties;
    };


    struct Edge {
        uint64_t left_id;
        uint64_t right_id;
        std::vector<std::string> labels;
        std::vector<Property> properties;
        EdgeDirection direction;
    };
}

#endif // BASE__IMPORT_AST_H_
