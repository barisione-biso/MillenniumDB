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

namespace import { namespace ast {

    namespace x3 = boost::spirit::x3;
    using namespace common::ast;

    struct Node {
        std::string name;
        std::vector<std::string> labels;
        std::vector<Property> properties;

        bool anonymous() const {
            return name[0] == '_';
        }
    };


    struct Edge {
        EdgeDirection direction;

        std::string left_name;
        std::string right_name;

        std::vector<std::string> labels;
        std::vector<Property> properties;

        bool left_anonymous() const {
            return left_name[0] == '_';
        }

        bool right_anonymous() const {
            return right_name[0] == '_';
        }
    };


    struct ImplicitEdge {
        EdgeDirection direction;

        // nesting_level = 1: left is the implicit node
        // nesting_level = 2: left is the edge
        // nesting_level = 3: left is the edge of the edge
        // and so on
        std::size_t nesting;

        std::size_t nesting_level() const {
            return nesting;
        }

        bool right_anonymous() const {
            return right_name[0] == '_';
        }

        std::string right_name;

        std::vector<std::string> labels;
        std::vector<Property> properties;
    };

    using ImportLine = boost::variant<Node, Edge, ImplicitEdge>;
}}

#endif // BASE__IMPORT_AST_H_
