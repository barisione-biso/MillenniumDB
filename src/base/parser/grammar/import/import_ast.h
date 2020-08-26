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
        bool anonymous;
        std::string name;
        std::vector<std::string> labels;
        std::vector<Property> properties;
    };


    struct Edge {
        EdgeDirection direction;

        bool left_anonymous;
        std::string left_name;

        bool right_anonymous;
        std::string right_name;

        std::vector<std::string> labels;
        std::vector<Property> properties;

    };


    struct ImplicitEdge {
        EdgeDirection direction;

        // nesting_level = 1: left is the implicit node
        // nesting_level = 2: left is the edge
        // nesting_level = 3: left is the edge of the edge
        // and so on
        int nesting_level;

        bool right_anonymous;
        std::string right_name;

        std::vector<std::string> labels;
        std::vector<Property> properties;
    };
}

#endif // BASE__IMPORT_AST_H_
