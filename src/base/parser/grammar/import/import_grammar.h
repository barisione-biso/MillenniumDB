#ifndef BASE__IMPORT_GRAMMAR_H_
#define BASE__IMPORT_GRAMMAR_H_

#include <boost/spirit/home/x3.hpp>

#include "import_ast.h"
#include "import_ast_adapted.h"
#include "base/parser/grammar/common/common_grammar.h"

namespace import_parser {
    namespace x3 = boost::spirit::x3;
    using namespace common_parser;

    using x3::blank;
    using x3::uint64;

    auto const skipper = blank;

    // Rules
    x3::rule<class node, import_ast::Node>
        node = "node";
    x3::rule<class edge, import_ast::Edge>
        edge = "edge";

    // Grammar
    auto const node_def =
        '(' >> uint64 >> ')'
        >> *label
        >> *property
        >> (eol|eoi);

    auto const edge_dir = lexeme[
        ("->" >> attr(import_ast::EdgeDirection::right)) |
        ("<-" >> attr(import_ast::EdgeDirection::left))
    ];

    auto const edge_def =
        '(' >> uint64 >> ')' >> edge_dir >> '(' >> uint64 >> ')'
        >> *label
        >> *property
        >> (eol|eoi);

    BOOST_SPIRIT_DEFINE(
        node,
        edge
    );
}

#endif // BASE__IMPORT_GRAMMAR_H_
