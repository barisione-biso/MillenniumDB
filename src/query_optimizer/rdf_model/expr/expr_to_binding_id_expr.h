#pragma once

#include <chrono>
#include <map>
#include <memory>
#include <random>
#include <boost/uuid/uuid_generators.hpp>

#include "base/ids/var_id.h"
#include "base/query/var.h"
#include "parser/query/expr/expr_visitor.h"
#include "execution/binding_id_iter/binding_id_expr/binding_id_expr.h"

namespace SPARQL {

// This visitor returns nullptr if condition is pushed outside
class Expr2BindingIdExpr : public ExprVisitor {
public:
    // Current time, used for NOW() and as seed for RAND()
    std::chrono::system_clock::time_point current_time;

    // Random number generator for RAND()
    std::default_random_engine rand_generator;
    std::uniform_real_distribution<float> distribution;

    // UUID generator for UUID() and STRUUID()
    boost::uuids::random_generator uuid_generator;

    const std::map<Var, VarId>& var2var_ids;

    std::unique_ptr<BindingIdExpr> current_binding_id_expr;

    Expr2BindingIdExpr(const std::map<Var, VarId>& var2var_ids) : var2var_ids(var2var_ids) {
        current_time   = std::chrono::system_clock::now();
        rand_generator = std::default_random_engine(current_time.time_since_epoch().count());
        distribution   = std::uniform_real_distribution<float>(0.0, 1.0);
    }

    void visit(SPARQL::ExprVar&)            override;
    void visit(SPARQL::ExprTerm&)           override;
    void visit(SPARQL::ExprEqual&)          override;
    void visit(SPARQL::ExprNotEqual&)       override;

    void visit(SPARQL::ExprNot&)            override;
    void visit(SPARQL::ExprUnaryMinus&)     override;
    void visit(SPARQL::ExprUnaryPlus&)      override;
    void visit(SPARQL::ExprMultiplication&) override;
    // void visit(SPARQL::ExprDivision&)       override;
    void visit(SPARQL::ExprAddition&)       override;
    void visit(SPARQL::ExprSubtraction&)    override;
    void visit(SPARQL::ExprAnd&)            override;
    void visit(SPARQL::ExprOr&)             override;
    
    // void visit(SPARQL::ExprLess&)           override;
    // void visit(SPARQL::ExprGreater&)        override;
    // void visit(SPARQL::ExprLessOrEqual&)    override;
    // void visit(SPARQL::ExprGreaterOrEqual&) override;

    // void visit(SPARQL::ExprIn&)             override;
    // void visit(SPARQL::ExprNotIn&)          override;

    // void visit(SPARQL::ExprAggAvg&)         override;
    // void visit(SPARQL::ExprAggCount&)       override;
    // void visit(SPARQL::ExprAggCountAll&)    override;
    // void visit(SPARQL::ExprAggGroupConcat&) override;
    // void visit(SPARQL::ExprAggMax&)         override;
    // void visit(SPARQL::ExprAggMin&)         override;
    // void visit(SPARQL::ExprAggSample&)      override;
    // void visit(SPARQL::ExprAggSum&)         override;

    // void visit(SPARQL::ExprAbs&)            override;
    // void visit(SPARQL::ExprBNode&)          override;
    void visit(SPARQL::ExprBound&)          override;
    // void visit(SPARQL::ExprCeil&)           override;
    void visit(SPARQL::ExprCoalesce&)       override;
    // void visit(SPARQL::ExprConcat&)         override; Needs temporary ObjectId (Literal/langtag)
    // void visit(SPARQL::ExprContains&)       override; Needs temporary ObjectId (Literal/langtag)
    // void visit(SPARQL::ExprDatatype&)       override; Needs temporary ObjectId (Literal/datatype)
    void visit(SPARQL::ExprDay&)            override;
    // void visit(SPARQL::ExprEncodeForUri&)   override; Needs temporary ObjectId (Literal/langtag)
    // void visit(SPARQL::ExprExists&)         override;
    // void visit(SPARQL::ExprFloor&)          override;
    void visit(SPARQL::ExprHours&)          override;
    void visit(SPARQL::ExprIf&)             override;
    void visit(SPARQL::ExprIRI&)            override;
    void visit(SPARQL::ExprIsBlank&)        override;
    void visit(SPARQL::ExprIsIRI&)          override;
    void visit(SPARQL::ExprIsLiteral&)      override;
    void visit(SPARQL::ExprIsNumeric&)      override;
    void visit(SPARQL::ExprIsURI&)          override;
    // void visit(SPARQL::ExprLang&)           override; Needs temporary ObjectId (Literal/langtag)
    // void visit(SPARQL::ExprLangMatches&)    override; Needs temporary ObjectId (Literal/langtag)
    void visit(SPARQL::ExprLCase&)          override; // Needs temporary ObjectId (Literal/langtag)
    void visit(SPARQL::ExprMD5&)            override;
    void visit(SPARQL::ExprMinutes&)        override;
    void visit(SPARQL::ExprMonth&)          override;
    // void visit(SPARQL::ExprNotExists&)      override;
    void visit(SPARQL::ExprNow&)            override;
    void visit(SPARQL::ExprRand&)           override;
    // void visit(SPARQL::ExprRegex&)          override; Needs temporary ObjectId (Literal/langtag)
    // void visit(SPARQL::ExprReplace&)        override; Needs temporary ObjectId (Literal/langtag)
    // void visit(SPARQL::ExprRound&)          override;
    void visit(SPARQL::ExprSameTerm&)       override;
    void visit(SPARQL::ExprSeconds&)        override;
    void visit(SPARQL::ExprSHA1&)           override;
    void visit(SPARQL::ExprSHA256&)         override;
    void visit(SPARQL::ExprSHA384&)         override;
    void visit(SPARQL::ExprSHA512&)         override;
    // void visit(SPARQL::ExprStrAfter&)       override; Needs temporary ObjectId (Literal/langtag)
    // void visit(SPARQL::ExprStrBefore&)      override; Needs temporary ObjectId (Literal/langtag)
    // void visit(SPARQL::ExprStrDT&)          override; Needs temporary ObjectId (Literal/langtag & IRI/prefix)
    // void visit(SPARQL::ExprStrEnds&)        override; Needs temporary ObjectId (Literal/langtag)
    // void visit(SPARQL::ExprStrLang&)        override; Needs temporary ObjectId (Literal/langtag)
    // void visit(SPARQL::ExprStrLen&)         override; Needs temporary ObjectId (Literal/langtag)
    // void visit(SPARQL::ExprStrStarts&)      override; Needs temporary ObjectId (Literal/langtag)
    void visit(SPARQL::ExprStrUUID&)        override;
    // void visit(SPARQL::ExprStr&)            override; Needs temporary ObjectId (Literal/langtag & IRI/prefix)
    void visit(SPARQL::ExprSubStr&)         override; // Needs temporary ObjectId (Literal/langtag)
    // void visit(SPARQL::ExprTimezone&)       override; Needs temporary ObjectId (Literal/datatype)
    void visit(SPARQL::ExprTZ&)             override;
    void visit(SPARQL::ExprUCase&)          override; // Needs temporary ObjectId (Literal/langtag)
    void visit(SPARQL::ExprURI&)            override;
    void visit(SPARQL::ExprUUID&)           override;
    void visit(SPARQL::ExprYear&)           override;
};
} // namespace SPARQL
