#include "expr_to_binding_id_expr.h"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "execution/binding_id_iter/binding_id_expr/sparql/binding_id_expr_and.h"
#include "execution/binding_id_iter/binding_id_expr/sparql/binding_id_expr_coalesce.h"
#include "execution/binding_id_iter/binding_id_expr/sparql/binding_id_expr_equals.h"
#include "execution/binding_id_iter/binding_id_expr/sparql/binding_id_expr_if.h"
#include "execution/binding_id_iter/binding_id_expr/sparql/binding_id_expr_is_blank.h"
#include "execution/binding_id_iter/binding_id_expr/sparql/binding_id_expr_is_iri.h"
#include "execution/binding_id_iter/binding_id_expr/sparql/binding_id_expr_is_literal.h"
#include "execution/binding_id_iter/binding_id_expr/sparql/binding_id_expr_is_numeric.h"
#include "execution/binding_id_iter/binding_id_expr/sparql/binding_id_expr_multiplication.h"
#include "execution/binding_id_iter/binding_id_expr/sparql/binding_id_expr_addition.h"
#include "execution/binding_id_iter/binding_id_expr/sparql/binding_id_expr_subtraction.h"
#include "execution/binding_id_iter/binding_id_expr/sparql/binding_id_expr_not.h"
#include "execution/binding_id_iter/binding_id_expr/sparql/binding_id_expr_or.h"
#include "execution/binding_id_iter/binding_id_expr/sparql/binding_id_expr_term.h"
#include "execution/binding_id_iter/binding_id_expr/sparql/binding_id_expr_unary_minus.h"
#include "execution/binding_id_iter/binding_id_expr/sparql/binding_id_expr_unary_plus.h"
#include "execution/binding_id_iter/binding_id_expr/sparql/binding_id_expr_var.h"
#include "execution/binding_id_iter/binding_id_expr/sparql/binding_id_expr_sha1.h"
#include "execution/binding_id_iter/binding_id_expr/sparql/binding_id_expr_sha256.h"
#include "execution/binding_id_iter/binding_id_expr/sparql/binding_id_expr_md5.h"
#include "execution/binding_id_iter/binding_id_expr/sparql/binding_id_expr_sha512.h"
#include "execution/binding_id_iter/binding_id_expr/sparql/binding_id_expr_sha384.h"
#include "execution/binding_id_iter/binding_id_expr/sparql/binding_id_expr_year.h"
#include "execution/binding_id_iter/binding_id_expr/sparql/binding_id_expr_month.h"
#include "execution/binding_id_iter/binding_id_expr/sparql/binding_id_expr_day.h"
#include "execution/binding_id_iter/binding_id_expr/sparql/binding_id_expr_hours.h"
#include "execution/binding_id_iter/binding_id_expr/sparql/binding_id_expr_minutes.h"
#include "execution/binding_id_iter/binding_id_expr/sparql/binding_id_expr_seconds.h"
#include "execution/binding_id_iter/binding_id_expr/sparql/binding_id_expr_tz.h"
#include "execution/binding_id_iter/binding_id_expr/sparql/binding_id_expr_same_term.h"
#include "execution/binding_id_iter/binding_id_expr/sparql/binding_id_expr_iri.h"
#include "execution/binding_id_iter/binding_id_expr/sparql/binding_id_expr_lcase.h"
#include "execution/binding_id_iter/binding_id_expr/sparql/binding_id_expr_ucase.h"
#include "execution/binding_id_iter/binding_id_expr/sparql/binding_id_expr_bound.h"
#include "execution/binding_id_iter/binding_id_expr/sparql/binding_id_expr_substr.h"
#include "parser/query/expr/sparql_exprs.h"
#include "query_optimizer/rdf_model/sparql_element_to_object_id.h"

using namespace std;
using namespace SPARQL;

void Expr2BindingIdExpr::visit(ExprVar& expr_var) {
    auto var_id             = var2var_ids.find(expr_var.var)->second;
    current_binding_id_expr = make_unique<BindingIdExprVar>(var_id);
}


void Expr2BindingIdExpr::visit(ExprTerm& expr_term) {
    SparqlElementToObjectId visitor(true);
    auto                    obj_id = visitor(expr_term.term);
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

    // (A != B) === NOT(A = B)
    current_binding_id_expr = make_unique<BindingIdExprEquals>(move(lhs), move(rhs));
    current_binding_id_expr = make_unique<BindingIdExprNot>(move(current_binding_id_expr));
}

void Expr2BindingIdExpr::visit(ExprNot& expr_not) {
    expr_not.expr->accept_visitor(*this);
    auto expr = move(current_binding_id_expr);

    current_binding_id_expr = make_unique<BindingIdExprNot>(move(expr));
}

void Expr2BindingIdExpr::visit(ExprUnaryMinus& expr_unary_minus) {
    expr_unary_minus.expr->accept_visitor(*this);
    auto expr = move(current_binding_id_expr);

    current_binding_id_expr = make_unique<BindingIdExprUnaryMinus>(move(expr));
}

void Expr2BindingIdExpr::visit(ExprUnaryPlus& expr_unary_plus) {
    expr_unary_plus.expr->accept_visitor(*this);
    auto expr = move(current_binding_id_expr);

    current_binding_id_expr = make_unique<BindingIdExprUnaryPlus>(move(expr));
}

void Expr2BindingIdExpr::visit(SPARQL::ExprMultiplication& expr_multiplication) {
    expr_multiplication.lhs->accept_visitor(*this);
    auto lhs = move(current_binding_id_expr);

    expr_multiplication.rhs->accept_visitor(*this);
    auto rhs = move(current_binding_id_expr);

    current_binding_id_expr = make_unique<BindingIdExprMultiplication>(move(lhs), move(rhs));
}

void Expr2BindingIdExpr::visit(SPARQL::ExprAddition& expr_addition) {
    expr_addition.lhs->accept_visitor(*this);
    auto lhs = move(current_binding_id_expr);

    expr_addition.rhs->accept_visitor(*this);
    auto rhs = move(current_binding_id_expr);

    current_binding_id_expr = make_unique<BindingIdExprAddition>(move(lhs), move(rhs));
}

void Expr2BindingIdExpr::visit(SPARQL::ExprSubtraction& expr_subtraction) {
    expr_subtraction.lhs->accept_visitor(*this);
    auto lhs = move(current_binding_id_expr);

    expr_subtraction.rhs->accept_visitor(*this);
    auto rhs = move(current_binding_id_expr);

    current_binding_id_expr = make_unique<BindingIdExprSubtraction>(move(lhs), move(rhs));
}

void Expr2BindingIdExpr::visit(ExprAnd& expr_and) {
    expr_and.lhs->accept_visitor(*this);
    auto lhs = move(current_binding_id_expr);

    expr_and.rhs->accept_visitor(*this);
    auto rhs = move(current_binding_id_expr);

    current_binding_id_expr = make_unique<BindingIdExprAnd>(move(lhs), move(rhs));
}

void Expr2BindingIdExpr::visit(ExprOr& expr_or) {
    expr_or.lhs->accept_visitor(*this);
    auto lhs = move(current_binding_id_expr);

    expr_or.rhs->accept_visitor(*this);
    auto rhs = move(current_binding_id_expr);

    current_binding_id_expr = make_unique<BindingIdExprOr>(move(lhs), move(rhs));
}

void Expr2BindingIdExpr::visit(SPARQL::ExprBound& expr_bound) {
    auto var_id = var2var_ids.find(expr_bound.var)->second;

    current_binding_id_expr = make_unique<BindingIdExprBound>(var_id);
}

void Expr2BindingIdExpr::visit(SPARQL::ExprCoalesce& expr_coalesce) {
    std::vector<std::unique_ptr<BindingIdExpr>> expr_list;
    for (auto& expr : expr_coalesce.exprs) {
        expr->accept_visitor(*this);
        expr_list.push_back(move(current_binding_id_expr));
    }

    current_binding_id_expr = make_unique<BindingIdExprCoalesce>(move(expr_list));
}

void Expr2BindingIdExpr::visit(SPARQL::ExprDay& expr_month) {
    expr_month.expr->accept_visitor(*this);

    auto expr = move(current_binding_id_expr);

    current_binding_id_expr = make_unique<BindingIdExprDay>(move(expr));
}

void Expr2BindingIdExpr::visit(SPARQL::ExprHours& expr_hours) {
    expr_hours.expr->accept_visitor(*this);

    auto expr = move(current_binding_id_expr);

    current_binding_id_expr = make_unique<BindingIdExprHours>(move(expr));
}

void Expr2BindingIdExpr::visit(SPARQL::ExprIf& expr_if) {
    expr_if.expr1->accept_visitor(*this);
    auto expr_cond = move(current_binding_id_expr);

    expr_if.expr2->accept_visitor(*this);
    auto expr_then = move(current_binding_id_expr);

    expr_if.expr3->accept_visitor(*this);
    auto expr_else = move(current_binding_id_expr);

    current_binding_id_expr = make_unique<BindingIdExprIf>(move(expr_cond), move(expr_then), move(expr_else));
}

void Expr2BindingIdExpr::visit(SPARQL::ExprIRI& expr_iri) {
    expr_iri.expr->accept_visitor(*this);

    auto expr = move(current_binding_id_expr);

    current_binding_id_expr = make_unique<BindingIdExprIRI>(move(expr), expr_iri.base_iri);
}

void Expr2BindingIdExpr::visit(SPARQL::ExprIsBlank& expr_is_blank) {
    expr_is_blank.expr->accept_visitor(*this);
    auto expr = move(current_binding_id_expr);

    current_binding_id_expr = make_unique<BindingIdExprIsBlank>(move(expr));
}
void Expr2BindingIdExpr::visit(SPARQL::ExprIsIRI& expr_is_iri) {
    expr_is_iri.expr->accept_visitor(*this);
    auto expr = move(current_binding_id_expr);

    current_binding_id_expr = make_unique<BindingIdExprIsIri>(move(expr));
}
void Expr2BindingIdExpr::visit(SPARQL::ExprIsLiteral& expr_is_literal) {
    expr_is_literal.expr->accept_visitor(*this);
    auto expr = move(current_binding_id_expr);

    current_binding_id_expr = make_unique<BindingIdExprIsLiteral>(move(expr));
}
void Expr2BindingIdExpr::visit(SPARQL::ExprIsNumeric& expr_is_numeric) {
    expr_is_numeric.expr->accept_visitor(*this);
    auto expr = move(current_binding_id_expr);

    current_binding_id_expr = make_unique<BindingIdExprIsNumeric>(move(expr));
}

void Expr2BindingIdExpr::visit(SPARQL::ExprIsURI& expr_is_uri) {
    expr_is_uri.expr->accept_visitor(*this);
    auto expr = move(current_binding_id_expr);

    // isURI is an alternate spelling for the isIRI function
    current_binding_id_expr = make_unique<BindingIdExprIsIri>(move(expr));
}

void Expr2BindingIdExpr::visit(SPARQL::ExprLCase& expr_lcase) {
    expr_lcase.expr->accept_visitor(*this);

    auto expr = move(current_binding_id_expr);

    current_binding_id_expr = make_unique<BindingIdExprLCase>(move(expr));
}

void Expr2BindingIdExpr::visit(SPARQL::ExprMD5& expr_md5) {
    expr_md5.expr->accept_visitor(*this);

    auto expr = move(current_binding_id_expr);

    current_binding_id_expr = make_unique<BindingIdExprMD5>(move(expr));
}

void Expr2BindingIdExpr::visit(SPARQL::ExprMinutes& expr_minutes) {
    expr_minutes.expr->accept_visitor(*this);

    auto expr = move(current_binding_id_expr);

    current_binding_id_expr = make_unique<BindingIdExprMinutes>(move(expr));
}

void Expr2BindingIdExpr::visit(SPARQL::ExprMonth& expr_month) {
    expr_month.expr->accept_visitor(*this);

    auto expr = move(current_binding_id_expr);

    current_binding_id_expr = make_unique<BindingIdExprMonth>(move(expr));
}

void Expr2BindingIdExpr::visit(SPARQL::ExprNow&) {
    // Each invocation of the NOW function returns exactly the same value.
    std::time_t t = std::chrono::system_clock::to_time_t(current_time);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&t), "%Y-%m-%dT%H:%M:%SZ");
    // Pack the datetime into an ObjectId
    ObjectId oid(ObjectId::MASK_DATETIME | DateTime::get_datetime_id(ss.str()));

    current_binding_id_expr = make_unique<BindingIdExprTerm>(oid);
}

void Expr2BindingIdExpr::visit(SPARQL::ExprRand&) {
    // Each invocation of the RAND function returns a new value.
    float f = distribution(rand_generator);

    current_binding_id_expr = make_unique<BindingIdExprTerm>(Conversions::pack_float(f));
}

void Expr2BindingIdExpr::visit(SPARQL::ExprSameTerm& expr_same_term) {
    expr_same_term.lhs->accept_visitor(*this);
    auto lhs = move(current_binding_id_expr);

    expr_same_term.rhs->accept_visitor(*this);
    auto rhs = move(current_binding_id_expr);

    current_binding_id_expr = make_unique<BindingIdExprSameTerm>(move(lhs), move(rhs));
}

void Expr2BindingIdExpr::visit(SPARQL::ExprSeconds& expr_seconds) {
    expr_seconds.expr->accept_visitor(*this);

    auto expr = move(current_binding_id_expr);

    current_binding_id_expr = make_unique<BindingIdExprSeconds>(move(expr));
}

void Expr2BindingIdExpr::visit(SPARQL::ExprSHA1& expr_sha1) {
    expr_sha1.expr->accept_visitor(*this);

    auto expr = move(current_binding_id_expr);

    current_binding_id_expr = make_unique<BindingIdExprSHA1>(move(expr));
}

void Expr2BindingIdExpr::visit(SPARQL::ExprSHA256& expr_sha256) {
    expr_sha256.expr->accept_visitor(*this);

    auto expr = move(current_binding_id_expr);

    current_binding_id_expr = make_unique<BindingIdExprSHA256>(move(expr));
}

void Expr2BindingIdExpr::visit(SPARQL::ExprSHA384& expr_sha384) {
    expr_sha384.expr->accept_visitor(*this);

    auto expr = move(current_binding_id_expr);

    current_binding_id_expr = make_unique<BindingIdExprSHA384>(move(expr));
}

void Expr2BindingIdExpr::visit(SPARQL::ExprSHA512& expr_sha512) {
    expr_sha512.expr->accept_visitor(*this);

    auto expr = move(current_binding_id_expr);

    current_binding_id_expr = make_unique<BindingIdExprSHA512>(move(expr));
}

void Expr2BindingIdExpr::visit(SPARQL::ExprStrUUID&) {
    // Each invocation of the STRUUID function returns a new value.
    boost::uuids::uuid uuid = uuid_generator();
    std::string uuid_str = boost::uuids::to_string(uuid);

    current_binding_id_expr = make_unique<BindingIdExprTerm>(Conversions::pack_string(uuid_str));
}

void Expr2BindingIdExpr::visit(SPARQL::ExprURI& expr_uri) {
    expr_uri.expr->accept_visitor(*this);

    auto expr = move(current_binding_id_expr);

    // URI is an alternate spelling for the IRI function
    current_binding_id_expr = make_unique<BindingIdExprIRI>(move(expr), expr_uri.base_iri);
}

void Expr2BindingIdExpr::visit(SPARQL::ExprSubStr& expr_substr) {
    expr_substr.expr1->accept_visitor(*this);
    auto expr_str = move(current_binding_id_expr);

    expr_substr.expr2->accept_visitor(*this);
    auto expr_start = move(current_binding_id_expr);

    if (expr_substr.expr3 == nullptr) {
        current_binding_id_expr = make_unique<BindingIdExprSubStr>(move(expr_str), move(expr_start));
    } else {
        expr_substr.expr3->accept_visitor(*this);
        auto expr_length = move(current_binding_id_expr);
        current_binding_id_expr = make_unique<BindingIdExprSubStr>(move(expr_str), move(expr_start), move(expr_length));
    }
}

void Expr2BindingIdExpr::visit(SPARQL::ExprTZ& expr_tz) {
    expr_tz.expr->accept_visitor(*this);

    auto expr = move(current_binding_id_expr);

    current_binding_id_expr = make_unique<BindingIdExprTZ>(move(expr));
}

void Expr2BindingIdExpr::visit(SPARQL::ExprUCase& expr_ucase) {
    expr_ucase.expr->accept_visitor(*this);

    auto expr = move(current_binding_id_expr);

    current_binding_id_expr = make_unique<BindingIdExprUCase>(move(expr));
}

void Expr2BindingIdExpr::visit(SPARQL::ExprUUID&) {
    // Each invocation of the UUID function returns a new value.
    boost::uuids::uuid uuid = uuid_generator();
    std::string uuid_str = "urn:uuid:" + boost::uuids::to_string(uuid);

    current_binding_id_expr = make_unique<BindingIdExprTerm>(Conversions::pack_iri(uuid_str));
}

void Expr2BindingIdExpr::visit(SPARQL::ExprYear& expr_year) {
    expr_year.expr->accept_visitor(*this);

    auto expr = move(current_binding_id_expr);

    current_binding_id_expr = make_unique<BindingIdExprYear>(move(expr));
}