#include "query_parser.h"

#include <iostream>

#include "base/parser/grammar/query/query_def.h"
#include "base/parser/grammar/query/printer/query_ast_printer.h"
#include "base/parser/grammar/manual_plan/manual_plan_def.h"
#include "base/parser/logical_plan/exceptions.h"
#include "base/parser/logical_plan/op/op_filter.h"
#include "base/parser/logical_plan/op/op_match.h"
#include "base/parser/logical_plan/op/op_select.h"
#include "base/parser/logical_plan/op/op_order_by.h"
#include "base/parser/logical_plan/op/op_group_by.h"
// #include "base/parser/logical_plan/op/op_optional.h"
#include "base/parser/logical_plan/op/visitors/check_var_names.h"

using namespace std;

unique_ptr<OpSelect> QueryParser::get_query_plan(query::ast::Root& ast) {
    unique_ptr<Op> op = make_unique<OpMatch>(ast.graph_pattern.pattern);

    if (ast.graph_pattern.optionals.size() > 0) {
        // TODO: `ast.graph_pattern.optionals` are ignored
        // TODO: crear OpOptional, dejar `op` listo para ser enchufado al OpFilter o al OpSelect
        // op = make_unique<OpOptional>(move(op), ast.graph_pattern.optionals);
    }

    uint_fast32_t limit = 0;
    if (ast.limit) {
        limit = ast.limit.get();
    }

    if (ast.where) {
        op = make_unique<OpFilter>(
            move(op),
            ast.where.get()
        );
    }

    if (ast.group_by) {
        auto group_by = ast.group_by.get();
        op = make_unique<OpGroupBy>(
            group_by,
            move(op),
            true // TODO: ascending from ast
        );
    }

    // TODO: si viene group_by + order by solo debería haber 1 OpGroupBy que sepa como ordenar?
    if (ast.order_by) {
        auto order_by = ast.order_by.get();
        op = make_unique<OpOrderBy>(
            order_by,
            move(op),
            true // TODO: ascending from ast
        );
    }

    return make_unique<OpSelect>(ast.selection, move(op), limit);
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
        check_query_plan(*res);
        return res;
    } else {
        cerr << "Error parsing at:\n" << string(iter, end);
        throw QueryParsingException();
    }
}


void QueryParser::check_query_plan(OpSelect& op_select) {
    auto check_var_names = CheckVarNames();
    check_var_names.visit(op_select);
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
