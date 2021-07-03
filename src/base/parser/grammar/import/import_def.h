#ifndef BASE__IMPORT_DEF_H_
#define BASE__IMPORT_DEF_H_

#include <boost/spirit/home/x3.hpp>

#include "import_ast.h"
#include "import_ast_adapted.h"
#include "import.h"
#include "base/parser/grammar/common/common_def.h"


namespace import {
    namespace parser {
        namespace x3 = boost::spirit::x3;
        using namespace common::parser;

        using x3::blank;
        using x3::uint64;

        auto const skipper = blank;

        // Rules
        x3::rule<class node, ast::Node> node = "node";
        x3::rule<class edge, ast::Edge> edge = "edge";
        x3::rule<class implicit_edge, ast::ImplicitEdge> implicit_edge = "implicit_edge";
        x3::rule<class import_line, ast::ImportLine> import_line = "import_line";

        // Grammar
        auto const end = (eol|eoi);

        auto const node_def =
            node_name
            >> *label
            >> *property
            >> end;

        auto const edge_dir =
        lexeme[
            ("->" >> attr(ast::EdgeDirection::right)) |
            ("<-" >> attr(ast::EdgeDirection::left))
        ];

        auto const edge_def =
            node_name
            >> edge_dir
            >> node_name
            >> *label
            >> *property
            >> end;

        auto const implicit_edge_def =
            lexeme[+char_('@')]
            >> edge_dir
            >> node_name
            >> *label
            >> *property
            >> end;

        auto import_line_def =
            (node_def | edge_def | implicit_edge_def);

        BOOST_SPIRIT_DEFINE(
            node,
            edge,
            implicit_edge,
            import_line
        )
    }

    parser::import_type import() {
        return parser::import_line;
    }
}

#endif // BASE__IMPORT_DEF_H_
