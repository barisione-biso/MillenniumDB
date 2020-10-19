#ifndef BASE__GRAMMAR_H_
#define BASE__GRAMMAR_H_

#include <boost/spirit/home/x3.hpp>

#include "query_ast.h"
#include "query_ast_adapted.h"
#include "query.h"
#include "base/parser/grammar/common/common_def.h"

namespace query {
    namespace parser {
        namespace x3 = boost::spirit::x3;
        using namespace common::parser;

        using x3::uint32;

        // Declare rules
        x3::rule<class root, ast::Root>
            root = "root";
        x3::rule<class select_item, ast::SelectItem>
            select_item = "select_item";
        x3::rule<class selection, std::vector<ast::SelectItem>>
            selection = "selection";
        x3::rule<class linear_pattern, ast::LinearPattern>
            linear_pattern = "linear_pattern";
        x3::rule<class node, ast::Node>
            node = "node";
        x3::rule<class edge, ast::Edge>
            edge = "edge";
        x3::rule<class condition, ast::Condition>
            condition = "condition";
        x3::rule<class statement, ast::Statement>
            statement = "statement";
        x3::rule<class formula, ast::Formula>
            formula = "formula";
        x3::rule<class step_formula, ast::StepFormula>
            step_formula = "step_formula";


        auto const comparator =
            lit("==") >> attr(ast::Comparator::EQ) |
            lit("<=") >> attr(ast::Comparator::LE) |
            lit(">=") >> attr(ast::Comparator::GE) |
            lit("!=") >> attr(ast::Comparator::NE) |
            lit('<')  >> attr(ast::Comparator::LT) |
            lit('>')  >> attr(ast::Comparator::GT);

        auto const connector =
            lexeme[no_case["or"]]  >> attr(ast::BinaryOp::Or) |
            lexeme[no_case["and"]] >> attr(ast::BinaryOp::And);

        auto const node_inside =
            -(var | node_name) >> *label >> -("{" >> -(property % ',') >> "}");

        auto const type =
            lexeme[no_case["=TYPE"]] >> '(' >> (var | node_name) >> ')';

        auto const edge_inside =
            -(var | node_name) >> *(type | label) >> -("{" >> -(property % ',') >> "}");

        auto const node_def =
            '(' >> node_inside >> ")";

        auto const edge_def =
            (-("-[" >> edge_inside >> ']') >> "->" >> attr(ast::EdgeDirection::right)) |
            ("<-" >> -('[' >> edge_inside >> "]-") >> attr(ast::EdgeDirection::left));

        auto const linear_pattern_def =
            node >> *(edge >> node);

        auto const statement_def =
            select_item >> comparator >> (select_item | value);

        auto const condition_def =
            -(no_case["NOT"] >> attr(true)) >>
            (
                statement |
                ('(' >> formula >> ')')
            );

        auto const step_formula_def =
            connector >> condition;

        auto const formula_def =
            condition >> *step_formula;

        auto const select_item_def =
            var >> -("." >> key);

        auto const selection_def =
            select_item % ',';

        auto explain_statement =
            no_case["explain"] >> attr(true) |
            attr(false);

        auto const select_statement =
            no_case["select"] >> ((lit('*') >> attr(std::vector<ast::SelectItem>()) ) | selection);

        auto const match_statement =
            no_case["match"] >> (linear_pattern % ',');


        auto const where_statement =
            no_case["where"] >> formula;

        auto const limit_statement =
            no_case["limit"] >> uint32;

        auto const root_def =
            explain_statement
            >> select_statement
            >> match_statement
            >> -(where_statement)
            >> -(limit_statement);

        BOOST_SPIRIT_DEFINE(
            root,
            selection,
            select_item,
            node,
            edge,
            linear_pattern,
            statement,
            formula,
            step_formula,
            condition
        );
    }

    parser::query_type query() {
        return parser::root;
    }
}

#endif // BASE__GRAMMAR_H_
