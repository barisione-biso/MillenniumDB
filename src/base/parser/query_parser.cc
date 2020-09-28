#include "query_parser.h"

#include <iostream>

#include "base/parser/grammar/query/query_def.h"
#include "base/parser/grammar/query/printer/query_ast_printer.h"
#include "base/parser/logical_plan/exceptions.h"
#include "base/parser/logical_plan/op/op_filter.h"
#include "base/parser/logical_plan/op/op_match.h"
#include "base/parser/logical_plan/op/op_select.h"
#include "base/parser/logical_plan/op/visitors/check_var_names.h"

using namespace std;

unique_ptr<OpSelect> QueryParser::get_query_plan(query::ast::QueryRoot& ast) {
    unique_ptr<Op> op_match = make_unique<OpMatch>(ast.graph_pattern);
    unique_ptr<Op> op_filter = make_unique<OpFilter>(ast.where, move(op_match));
    uint_fast32_t limit = 0;
    if (ast.limit) {
        limit = ast.limit.get();
    }
    return make_unique<OpSelect>(ast.selection, move(op_filter), limit);
}


unique_ptr<OpSelect> QueryParser::get_query_plan(string& query) {
    auto iter = query.begin();
    auto end = query.end();

    query::ast::QueryRoot ast;
    bool r = phrase_parse(iter, end, query::parser::query_root, query::parser::skipper, ast);
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


// manual_plan_ast::Root QueryParser::get_manual_plan(string& query) {
//     cout << "ManualPlan:\n" << query << "\n";
//     auto iter = query.begin();
//     auto end = query.end();

//     manual_plan_ast::Root manual_plan;
//     bool r = phrase_parse(iter, end, manual_plan::parser::root, manual_plan::parser::skipper, manual_plan);
//     if (r && iter == end) { // parsing succeeded
//         return manual_plan;
//     } else {
//         cout << "ManualPlan failed\n";
//         throw QueryParsingException();
//     }
// }
