#ifndef BASE__GRAMMAR_H_
#define BASE__GRAMMAR_H_

#include <boost/spirit/home/x3.hpp>

#include "base/parser/grammar/query/query_ast.h"
#include "base/parser/grammar/query/query_ast_adapted.h"
#include "base/parser/grammar/common/common_grammar.h"

namespace query_parser {
    namespace x3 = boost::spirit::x3;
    using namespace common_parser;

    using x3::int32;

    // Declare rules
    x3::rule<class root, query_ast::Root>
        root = "root";
    x3::rule<class element, query_ast::Element>
        element = "element";
    x3::rule<class linear_pattern, query_ast::LinearPattern>
        linear_pattern = "linear_pattern";
    x3::rule<class node, query_ast::Node>
        node = "node";
    x3::rule<class edge, query_ast::Edge>
        edge = "edge";
    x3::rule<class condition, query_ast::Condition>
        condition = "condition";
    x3::rule<class statement, query_ast::Statement>
        statement = "statement";
    x3::rule<class formula, query_ast::Formula>
        formula = "formula";

    auto const comparator =
        lit("==") >> attr(query_ast::EQ()) |
        lit("<=") >> attr(query_ast::LE()) |
        lit(">=") >> attr(query_ast::GE()) |
        lit("!=") >> attr(query_ast::NE()) |
        lit('<')  >> attr(query_ast::LT()) |
        lit('>')  >> attr(query_ast::GT());

    auto const connector =
        lexeme[no_case["or"]]  >> attr(query_ast::Or()) |
        lexeme[no_case["and"]] >> attr(query_ast::And());

    auto const func =
        lexeme[+(alnum)];

    auto const nomen =
        -var >> *label >> -("{" >> -(property % ',') >> "}");

    auto const node_def =
        '(' >> nomen >> ")";

    auto const edge_def =
        (-("-[" >> nomen >> ']') >> "->" >> attr(query_ast::EdgeDirection::right)) |
        ("<-" >> -('[' >> nomen >> "]-") >> attr(query_ast::EdgeDirection::left));

    auto const linear_pattern_def =
        node >> *(edge >> node) >> ((no_case["ON"] >> common_parser::string) | attr(std::string()) );

    auto const selection =
        lit('*') >> attr(query_ast::All()) | (element % ',');

    auto const statement_def =
        (element | var) >> comparator >> (element | var | value);

    auto const condition_def =
        -(no_case["NOT"] >> attr(true)) >>
        (
            statement |
            ('(' >> formula >> ')')
        );

    auto const formula_def =
        condition >> *(connector >> condition);

    auto const match_statement =
        no_case["match"] >> (linear_pattern % ',');

    auto const select_statement =
        no_case["select"] >> selection;

    auto const where_statement =
        no_case["where"] >> formula;

    auto const limit_statement =
        no_case["limit"] >> int32;

    auto const root_def =
        (no_case["explain"] >> attr(true)) >> select_statement >> match_statement >> -(where_statement) >> -(limit_statement) |
        (attr(false)) >> select_statement >> match_statement >> -(where_statement) >> -(limit_statement);

    auto const element_def =
        var >> '.' >> key;

    BOOST_SPIRIT_DEFINE(
        root,
        element,
        linear_pattern,
        node,
        edge,
        condition,
        statement,
        formula
    );
}

#endif // BASE__GRAMMAR_H_
