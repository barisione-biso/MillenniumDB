#include "expr_to_binding_id_expr.h"

#include "execution/binding_id_iter/binding_id_expr/sparql/binding_id_expr_equals.h"
#include "execution/binding_id_iter/binding_id_expr/sparql/binding_id_expr_not_equals.h"
#include "execution/binding_id_iter/binding_id_expr/sparql/binding_id_expr_term.h"
#include "execution/binding_id_iter/binding_id_expr/sparql/binding_id_expr_var.h"
#include "query_optimizer/rdf_model/sparql_element_to_object_id.h"
#include "parser/query/expr/sparql_exprs.h"

using namespace std;
using namespace SPARQL;

void Expr2BindingIdExpr::visit(ExprVar& expr_var) {
    auto var_id = var2var_ids.find(expr_var.var)->second;
    current_binding_id_expr = make_unique<BindingIdExprVar>(var_id);
}


void Expr2BindingIdExpr::visit(ExprTerm& expr_term) {
    SparqlElementToObjectId visitor(false);
    auto obj_id = visitor(expr_term.term);
    // TODO: necesito que entregue un ID válido si es externo que no esta en la BD ("Un ejemplo largo")
    current_binding_id_expr = make_unique<BindingIdExprTerm>(obj_id);
}


void Expr2BindingIdExpr::visit(ExprEqual& expr_equal) {
    expr_equal.lhs->accept_visitor(*this);
    auto lhs = move(current_binding_id_expr);

    expr_equal.rhs->accept_visitor(*this);
    auto rhs = move(current_binding_id_expr);

    current_binding_id_expr = make_unique<BindingIdExprEquals>(move(lhs), move(rhs));
}


void Expr2BindingIdExpr::visit(ExprNotEqual& expr_not_equal) {
    expr_not_equal.lhs->accept_visitor(*this);
    auto lhs = move(current_binding_id_expr);

    expr_not_equal.rhs->accept_visitor(*this);
    auto rhs = move(current_binding_id_expr);

    current_binding_id_expr = make_unique<BindingIdExprNotEquals>(move(lhs), move(rhs));
}
