#include "query_parser.h"

#include <iostream>

#include "base/parser/grammar/manual_plan/manual_plan_def.h"
#include "base/parser/grammar/query/query_def.h"
#include "base/parser/logical_plan/exceptions.h"
#include "base/parser/logical_plan/op/op_filter.h"
#include "base/parser/logical_plan/op/op_graph_pattern_root.h"
#include "base/parser/logical_plan/op/op_group_by.h"
#include "base/parser/logical_plan/op/op_match.h"
#include "base/parser/logical_plan/op/op_optional.h"
#include "base/parser/logical_plan/op/op_order_by.h"
#include "base/parser/logical_plan/op/op_select.h"
#include "base/parser/logical_plan/op/op_distinct.h"
#include "base/parser/logical_plan/op/visitors/check_var_names.h"
#include "base/parser/logical_plan/op/visitors/check_well_designed.h"
#include "base/parser/logical_plan/op/visitors/optimize_tree.h"

using namespace std;

unique_ptr<OpSelect> QueryParser::get_query_plan(query::ast::Root& ast) {
    unique_ptr<Op> op;

    uint_fast32_t anon_count = 0;
    if (ast.graph_pattern.optionals.size() > 0) {
        op = make_unique<OpOptional>(ast.graph_pattern, &anon_count);
    } else {
        op = make_unique<OpMatch>(ast.graph_pattern.pattern, &anon_count);
    }

    op = make_unique<OpGraphPatternRoot>(std::move(op));

    uint_fast32_t limit = 0;
    if (ast.limit) {
        limit = ast.limit.get();
    }

    if (ast.where) {
        op = make_unique<OpFilter>(move(op), ast.where.get());
    }

    if (ast.group_by) {
        op = make_unique<OpGroupBy>(move(op), ast.group_by.get());
    }

    // TODO: si viene group_by + order by solo debería haber 1 OpGroupBy que sepa como ordenar?
    if (ast.order_by) {
        op = make_unique<OpOrderBy>(move(op), ast.order_by.get());
    }

    // TODO: create op distinct if necesary
    if (ast.select.distinct) {
        // TODO:
        op = make_unique<OpDistinct>(move(op)); // tal vez hay que pasarle mas cosas?
    }

    // OpSelect
    //     |
    // OpDistinct
    //     |
    // OpFilter
    //     |
    // OpMatch
    //     |
    //     - OpProperty
    //     - OpLabel
    //     - OpConnection

    return make_unique<OpSelect>(move(op), ast.select.selection, limit);
}


unique_ptr<OpSelect> QueryParser::get_query_plan(string& query) {
    auto iter = query.begin();
    auto end = query.end();

    query::ast::Root ast;
    bool r = phrase_parse(iter, end, query::parser::root, query::parser::skipper, ast);
    if (r && iter == end) { // parsing succeeded
        if (ast.explain) {
            QueryAstPrinter printer(cout);
            printer(ast);
        }
        auto res = QueryParser::get_query_plan(ast);
        if (ast.explain) {
            cout << *res << "\n";
        }
        check_query_plan(*res);
        return res;
    } else {
        cerr << "Error parsing at:\n" << string(iter, end);
        throw QueryParsingException();
    }
}


void QueryParser::check_query_plan(OpSelect& op_select) {
    CheckVarNames check_var_names;
    check_var_names.visit(op_select);

    CheckWellDesigned check_well_designed;
    check_well_designed.visit(op_select);

    OptimizeTree optimize_tree;
    optimize_tree.visit(op_select);
}


manual_plan::ast::ManualRoot QueryParser::get_manual_plan(string& query) {
    auto iter = query.begin();
    auto end = query.end();

    manual_plan::ast::ManualRoot manual_plan;
    bool r = phrase_parse(iter, end, manual_plan::parser::manual_root, manual_plan::parser::skipper, manual_plan);
    if (r && iter == end) { // parsing succeeded
        return manual_plan;
    } else {
        cout << "\nManualPlan failed\n";
        throw QueryParsingException();
    }
}
