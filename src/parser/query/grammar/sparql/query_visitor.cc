#include "query_visitor.h"

#include "base/exceptions.h"
#include "parser/query/expr/sparql_exprs.h"
#include "parser/query/op/sparql/ops.h"
#include "parser/query/paths/path_alternatives.h"
#include "parser/query/paths/path_sequence.h"
#include "parser/query/paths/path_atom.h"
#include "parser/query/paths/path_kleene_star.h"
#include "parser/query/paths/path_optional.h"
#include "parser/query/paths/path_negated_set.h"

using namespace SPARQL;
using antlrcpp::Any;

QueryVisitor::QueryVisitor() {
    prefix_iris_map.insert({"xsd", "http://www.w3.org/2001/XMLSchema#"});
}

Any QueryVisitor::visitConstructQuery(SparqlParser::ConstructQueryContext*) {
    throw NotSupportedException("Construct query");
}

Any QueryVisitor::visitDescribeQuery(SparqlParser::DescribeQueryContext*) {
    throw NotSupportedException("Describe query");
}

Any QueryVisitor::visitAskQuery(SparqlParser::AskQueryContext*) {
    throw NotSupportedException("Ask query");
}

Any QueryVisitor::visitUpdateCommand(SparqlParser::UpdateCommandContext*) {
    throw NotSupportedException("Update command");
}

// Root parser rule
Any QueryVisitor::visitQuery(SparqlParser::QueryContext* ctx) {
    visitChildren(ctx);
    if (current_op == nullptr) {
        throw QueryParsingException("Empty query");
    }
    return 0;
}


Any QueryVisitor::visitBaseDecl(SparqlParser::BaseDeclContext* ctx) {
    base_iri = ctx->IRIREF()->getText();
    base_iri = base_iri.substr(1, base_iri.size() - 2);
    return 0;
}


Any QueryVisitor::visitPrefixDecl(SparqlParser::PrefixDeclContext* ctx) {
    std::string prefix = ctx->PNAME_NS()->getText();
    prefix             = prefix.substr(0, prefix.size() - 1);
    std::string iri    = ctx->IRIREF()->getText();
    iri                = iri.substr(1, iri.size() - 2);

    if (prefix_iris_map.contains(prefix)) {
        throw QuerySemanticException("Multiple prefix declarations for prefix: '" + prefix + "'");
    }

    prefix_iris_map.insert({prefix, iri});
    return 0;
}


Any QueryVisitor::visitSolutionModifier(SparqlParser::SolutionModifierContext* ctx) {
    // LIMIT and OFFSET
    auto limit_offset_clauses = ctx->limitOffsetClauses();
    if (limit_offset_clauses) {
        auto limit_clause = limit_offset_clauses->limitClause();
        if (limit_clause) {
            limit = std::stoull(limit_clause->INTEGER()->getText());
        }
        auto offset_clause = limit_offset_clauses->offsetClause();
        if (offset_clause) {
            offset = std::stoull(offset_clause->INTEGER()->getText());
        }
    }
    // ORDER BY
    auto order_clause = ctx->orderClause();
    if (order_clause) {
        visit(order_clause);
        current_op = std::make_unique<OpOrderBy>(std::move(current_op), std::move(order_by_items), std::move(order_by_ascending));
    }
    return 0;
}


Any QueryVisitor::visitOrderClause(SparqlParser::OrderClauseContext* ctx) {
    for (auto& oc : ctx->orderCondition()) {
        if (oc->var()) {
            order_by_items.emplace_back(oc->var()->getText().substr(1));
            order_by_ascending.push_back(true);
        } else {
            throw QuerySemanticException("Unsupported ORDER BY condition: '" + oc->getText() + "'");
        }
    }
    return 0;
}


Any QueryVisitor::visitSelectQuery(SparqlParser::SelectQueryContext* ctx) {
    visit(ctx->whereClause());
    visit(ctx->solutionModifier());
    visit(ctx->selectClause());
    // TODO: consider datasetClause
    return 0;
}


Any QueryVisitor::visitSelectClause(SparqlParser::SelectClauseContext* ctx) {
    visitChildren(ctx);
    if (ctx->ASTERISK() != nullptr) {
        for (auto& var : current_op->get_vars()) {
            // Prevent storing blank nodes
            if (var.name.find(':') == std::string::npos) {
                select_variables.push_back(var);
            }
        }
    }
    // We treat both select modifiers as the same (DISTINCT and REDUCED)
    current_op = std::make_unique<OpSelect>(std::move(current_op),
                                            std::move(select_variables),
                                            ctx->selectModifier() != nullptr,
                                            limit,
                                            offset);
    return 0;
}


Any QueryVisitor::visitSelectSingleVariable(SparqlParser::SelectSingleVariableContext* ctx) {
    select_variables.push_back(Var(ctx->var()->getText().substr(1)));
    return 0;
}


Any QueryVisitor::visitWhereClause(SparqlParser::WhereClauseContext* ctx) {
    visit(ctx->groupGraphPattern());
    assert(current_op != nullptr);
    current_op = std::make_unique<OpWhere>(std::move(current_op));
    return 0;
}


Any QueryVisitor::visitGroupGraphPatternSub(SparqlParser::GroupGraphPatternSubContext* ctx) {
    // 1. Visit the outermost graph pattern
    if (ctx->triplesBlock()) {
        visit(ctx->triplesBlock());
    } else {
        current_op = std::make_unique<OpUnitTable>();
    }
    // add empty list to the stack
    current_filters.push({});

    // 2. Visit special scopes (OPTIONAL, UNION, FILTER, etc.) and other basic graph patterns
    for (auto group_graph_pattern_sub_list : ctx->groupGraphPatternSubList()) {
        visit(group_graph_pattern_sub_list->graphPatternNotTriples());

        if (group_graph_pattern_sub_list->triplesBlock()) {
            std::unique_ptr<Op> lhs_op = std::move(current_op);
            visit(group_graph_pattern_sub_list->triplesBlock());
            // If lhs_op is not an OpUnitTable, we need to join it with the current_op
            if (dynamic_cast<OpUnitTable*>(lhs_op.get()) == nullptr) {
                current_op = std::make_unique<OpJoin>(std::move(lhs_op), std::move(current_op));
            }
        }
    }

    std::vector<std::unique_ptr<Expr>>& filters = current_filters.top();
    if (filters.size() > 0) {
        current_op = std::make_unique<OpFilter>(std::move(current_op), std::move(filters));
    }
    current_filters.pop();
    return 0;
}


Any QueryVisitor::visitGroupOrUnionGraphPattern(SparqlParser::GroupOrUnionGraphPatternContext* ctx) {
    std::unique_ptr<Op> lhs_op = std::move(current_op);

    std::vector<std::unique_ptr<Op>> union_ops;
    for (auto& ggp_item : ctx->groupGraphPattern()) {
        visit(ggp_item);
        union_ops.push_back(std::move(current_op));
    }
    // If the union has only one element, we don't need to create a union operator
    if (union_ops.size() == 1) {
        current_op = std::move(union_ops[0]);
    } else {
        current_op = std::make_unique<OpUnion>(std::move(union_ops));
    }
    // If lhs_op is not an OpUnitTable, we need to join it with the current_op
    if (dynamic_cast<OpUnitTable*>(lhs_op.get()) == nullptr) {
        current_op = std::make_unique<OpJoin>(std::move(lhs_op), std::move(current_op));
    }
    return 0;
}


Any QueryVisitor::visitOptionalGraphPattern(SparqlParser::OptionalGraphPatternContext* ctx) {
    std::unique_ptr<Op> lhs_op = std::move(current_op);

    visit(ctx->groupGraphPattern());
    current_op = std::make_unique<OpOptional>(std::move(lhs_op), std::move(current_op));
    return 0;
}


Any QueryVisitor::visitMinusGraphPattern(SparqlParser::MinusGraphPatternContext* ctx) {
    std::unique_ptr<Op> lhs_op = std::move(current_op);

    visit(ctx->groupGraphPattern());
    current_op = std::make_unique<OpMinus>(std::move(lhs_op), std::move(current_op));
    return 0;
}


Any QueryVisitor::visitServiceGraphPattern(SparqlParser::ServiceGraphPatternContext* ctx) {
    std::unique_ptr<Op> lhs_op = std::move(current_op);

    visit(ctx->groupGraphPattern());
    bool silent = ctx->SILENT() != nullptr;
    auto voi = ctx->varOrIRI();
    if (voi->iri()) {
        // It's an IRI
        current_op = std::make_unique<OpService>(silent,
                                                 Iri(iriCtxToString(voi->iri())),
                                                 std::move(current_op));
    } else {
        // It's a variable
        current_op = std::make_unique<OpService>(silent,
                                                 Var(voi->var()->getText().substr(1)),
                                                 std::move(current_op));
    }
    // If lhs_op is not an OpUnitTable, we need to join it with the current_op
    if (dynamic_cast<OpUnitTable*>(lhs_op.get()) == nullptr) {
        current_op = std::make_unique<OpJoin>(std::move(lhs_op), std::move(current_op));
    }
    return 0;
}


Any QueryVisitor::visitGraphGraphPattern(SparqlParser::GraphGraphPatternContext* ctx) {
    std::unique_ptr<Op> lhs_op = std::move(current_op);

    visit(ctx->groupGraphPattern());
    if (ctx->varOrIRI()) {
        // It's an IRI
        current_op = std::make_unique<OpGraph>(Iri(iriCtxToString(ctx->varOrIRI()->iri())),
                                               std::move(current_op));
    } else {
        // It's a variable
        current_op = std::make_unique<OpGraph>(Var(ctx->varOrIRI()->var()->getText().substr(1)),
                                               std::move(current_op));
    }
    // If lhs_op is not an OpUnitTable, we need to join it with the current_op
    if (dynamic_cast<OpUnitTable*>(lhs_op.get()) == nullptr) {
        current_op = std::make_unique<OpJoin>(std::move(lhs_op), std::move(current_op));
    }
    return 0;
}


Any QueryVisitor::visitFilter(SparqlParser::FilterContext* ctx) {
    visit(ctx->constraint());
    current_filters.top().push_back(std::move(current_expr));
    return 0;
}


Any QueryVisitor::visitMultiplicativeExpression(SparqlParser::MultiplicativeExpressionContext* ctx) {
    visit(ctx->unaryExpression(0));
    assert(current_expr != nullptr);


    assert(ctx->op.size() == ctx->unaryExpression().size() - 1);
    for (size_t i = 1; i < ctx->unaryExpression().size(); i++) {
        std::unique_ptr<Expr> lhs = std::move(current_expr);
        visit(ctx->unaryExpression(i));
        assert(current_expr != nullptr);

        switch(ctx->op[i-1]->getType()) {
            case SparqlParser::ASTERISK:
                current_expr = std::make_unique<ExprMultiplication>(std::move(lhs), std::move(current_expr));
                break;
            case SparqlParser::DIVIDE:
                current_expr = std::make_unique<ExprDivision>(std::move(lhs), std::move(current_expr));
                break;
            default:
                throw QuerySemanticException("Unhandled multiplicative expression");
        }
    }
    return 0;
}


Any QueryVisitor::visitPrimaryExpression(SparqlParser::PrimaryExpressionContext* ctx) {
    if (ctx->expression()) {
        visit(ctx->expression());
    } else if (ctx->builtInCall()) {
        visit(ctx->builtInCall());
    } else if (ctx->iriOrFunction()) {
        if (ctx->iriOrFunction()->argList()) {
            throw NotSupportedException("IRI function"); // TODO: support?
        } else {
            current_expr = std::make_unique<ExprTerm>(SparqlElement(
                Iri(iriCtxToString(ctx->iriOrFunction()->iri()))
            ));
        }
    } else if (ctx->rdfLiteral()) {
        auto literal_str_with_quotes = ctx->rdfLiteral()->string()->getText();
        auto literal_str = literal_str_with_quotes.substr(1, literal_str_with_quotes.size() - 2);
        if (ctx->rdfLiteral()->LANGTAG()) {
            // string with language
            current_expr = std::make_unique<ExprTerm>(SparqlElement(
                LiteralLanguage(literal_str, ctx->rdfLiteral()->LANGTAG()->getText().substr(1))
            ));
        } else if (ctx->rdfLiteral()->iri()) {
            // string with datatype
            auto datatype = iriCtxToString(ctx->rdfLiteral()->iri());

            if (datatype == "http://www.w3.org/2001/XMLSchema#string") {
                current_expr = std::make_unique<ExprTerm>(SparqlElement(
                    Literal(literal_str)
                ));
            // TODO: detect more supported datatypes
            } else {
                current_expr = std::make_unique<ExprTerm>(SparqlElement(
                    LiteralDatatype(literal_str, datatype)
                ));
            }
        } else {
            // string simple
            current_expr = std::make_unique<ExprTerm>(SparqlElement(
                Literal(literal_str)
            ));
        }
    } else if (ctx->numericLiteral()) {
        // TODO: detect if it is int, float or decimal?
        current_expr = std::make_unique<ExprTerm>(SparqlElement(
            Decimal(ctx->numericLiteral()->getText())
        ));
    } else if (ctx->booleanLiteral()) {
        current_expr = std::make_unique<ExprTerm>(SparqlElement(
            ctx->booleanLiteral()->TRUE() != nullptr
        ));
    } else if (ctx->var()) {
        current_expr = std::make_unique<ExprVar>(ctx->var()->getText().substr(1));
    }
    else {
        throw QuerySemanticException("Unhandled primary expression");
    }
    return 0;
}


Any QueryVisitor::visitUnaryExpression(SparqlParser::UnaryExpressionContext* ctx) {
    visit(ctx->primaryExpression());
    assert(current_expr != nullptr);
    if (ctx->op != nullptr) {
        switch(ctx->op->getType()) {
            case SparqlParser::PLUS_SIGN:
                current_expr = std::make_unique<ExprUnaryPlus>(std::move(current_expr));
                break;
            case SparqlParser::MINUS_SIGN:
                current_expr = std::make_unique<ExprUnaryMinus>(std::move(current_expr));
                break;
            case SparqlParser::NEGATION:
                current_expr = std::make_unique<ExprNot>(std::move(current_expr));
                break;
            default:
                throw QuerySemanticException("Unhandled additive expression");
        }
    }
    return 0;
}


Any QueryVisitor::visitAdditiveExpression(SparqlParser::AdditiveExpressionContext* ctx) {
    visit(ctx->multiplicativeExpression());
    assert(current_expr != nullptr);

    for (size_t i = 0; i < ctx->rhsAdditiveExpression().size(); i++) {
        std::unique_ptr<Expr> additive_lhs = std::move(current_expr);

        auto current_additive_expr = ctx->rhsAdditiveExpression(i);
        auto current_additive_sub_expr = current_additive_expr->rhsAdditiveExpressionSub();

        bool minus = false;
        std::unique_ptr<Expr> multiplicative_rhs;
        if (current_additive_sub_expr->numericLiteralPositive()) {
            multiplicative_rhs = std::make_unique<ExprTerm>(SparqlElement(
                Decimal(current_additive_sub_expr->numericLiteralPositive()->getText().substr(1))
            ));
        } else if (current_additive_sub_expr->numericLiteralNegative()) {
            minus = true;
            multiplicative_rhs = std::make_unique<ExprTerm>(SparqlElement(
                Decimal(current_additive_sub_expr->numericLiteralNegative()->getText().substr(1))
            ));
        } else if (current_additive_sub_expr->PLUS_SIGN()) {
            visit(current_additive_sub_expr->multiplicativeExpression());
            assert(current_expr != nullptr);
            multiplicative_rhs = std::move(current_expr);
        } else if (current_additive_sub_expr->MINUS_SIGN()) {
            minus = true;
            visit(current_additive_sub_expr->multiplicativeExpression());
            assert(current_expr != nullptr);
            multiplicative_rhs = std::move(current_expr);
        }

        for (size_t j = 0; j < current_additive_expr->unaryExpression().size(); j++) {
            visit(current_additive_expr->unaryExpression(j));
            assert(current_expr != nullptr);

            if (current_additive_expr->op[j]->getType() == SparqlParser::ASTERISK) {
                multiplicative_rhs = std::make_unique<ExprMultiplication>(std::move(multiplicative_rhs), std::move(current_expr));
            } else {
                multiplicative_rhs = std::make_unique<ExprDivision>(std::move(multiplicative_rhs), std::move(current_expr));
            }
        }

        if (minus) {
            current_expr = std::make_unique<ExprSubtraction>(std::move(additive_lhs), std::move(multiplicative_rhs));
        } else {
            current_expr = std::make_unique<ExprAddition>(std::move(additive_lhs), std::move(multiplicative_rhs));
        }
    }
    return 0;
}


Any QueryVisitor::visitRelationalExpression(SparqlParser::RelationalExpressionContext* ctx) {
    visit(ctx->additiveExpression(0));
    assert(current_expr != nullptr);

    if (ctx->IN() != nullptr) {
        visit(ctx->additiveExpression()[0]);
        auto lhs_expr = std::move(current_expr);
        // IN or NOT IN
        std::vector<std::unique_ptr<Expr>> expr_list;

        for (auto expr_ctx : ctx->expressionList()->expression()) {
            visit(expr_ctx);
            expr_list.push_back(std::move(current_expr));
        }

        if (ctx->NOT() != nullptr) {
            current_expr = std::make_unique<ExprNotIn>(std::move(lhs_expr), std::move(expr_list));
        } else {
            current_expr = std::make_unique<ExprIn>(std::move(lhs_expr), std::move(expr_list));
        }
    } else if (ctx->op != nullptr) {
        std::unique_ptr<Expr> lhs = std::move(current_expr);
        visit(ctx->additiveExpression(1));
        assert(current_expr != nullptr);
        // op=('='|'!='|'<'|'>'|'<='|'>=')
        switch(ctx->op->getType()) {
            case SparqlParser::EQUAL:
                current_expr = std::make_unique<ExprEqual>(std::move(lhs), std::move(current_expr));
                break;
            case SparqlParser::NOT_EQUAL:
                current_expr = std::make_unique<ExprNotEqual>(std::move(lhs), std::move(current_expr));
                break;
            case SparqlParser::LESS:
                current_expr = std::make_unique<ExprLess>(std::move(lhs), std::move(current_expr));
                break;
            case SparqlParser::GREATER:
                current_expr = std::make_unique<ExprGreater>(std::move(lhs), std::move(current_expr));
                break;
            case SparqlParser::LESS_EQUAL:
                current_expr = std::make_unique<ExprLessOrEqual>(std::move(lhs), std::move(current_expr));
                break;
            case SparqlParser::GREATER_EQUAL:
                current_expr = std::make_unique<ExprGreaterOrEqual>(std::move(lhs), std::move(current_expr));
                break;
            default:
                throw QuerySemanticException("Unhandled relational expression");
        }
    }
    return 0;
}


Any QueryVisitor::visitConditionalAndExpression(SparqlParser::ConditionalAndExpressionContext* ctx) {
    visit(ctx->relationalExpression(0));
    assert(current_expr != nullptr);

    for (size_t i = 1; i < ctx->relationalExpression().size(); i++) {
        std::unique_ptr<Expr> lhs = std::move(current_expr);
        visit(ctx->relationalExpression(i));
        assert(current_expr != nullptr);
        current_expr = std::make_unique<ExprAnd>(std::move(lhs), std::move(current_expr));
    }
    return 0;
}


Any QueryVisitor::visitConditionalOrExpression(SparqlParser::ConditionalOrExpressionContext* ctx) {
    visit(ctx->conditionalAndExpression(0));
    assert(current_expr != nullptr);

    for (size_t i = 1; i < ctx->conditionalAndExpression().size(); i++) {
        std::unique_ptr<Expr> lhs = std::move(current_expr);
        visit(ctx->conditionalAndExpression(i));
        assert(current_expr != nullptr);
        current_expr = std::make_unique<ExprOr>(std::move(lhs), std::move(current_expr));
    }
    return 0;
}


Any QueryVisitor::visitBuiltInCall(SparqlParser::BuiltInCallContext* ctx) {
    if (ctx->aggregate()) {
        visit(ctx->aggregate());
    }
    else if (ctx->STR()) {
        visit(ctx->expression(0));
        current_expr = std::make_unique<ExprStr>(std::move(current_expr));
    }
    else if (ctx->LANG()) {
        visit(ctx->expression(0));
        current_expr = std::make_unique<ExprLang>(std::move(current_expr));
    }
    else if (ctx->LANGMATCHES()) {
        visit(ctx->expression(0));
        auto expr0 = std::move(current_expr);
        visit(ctx->expression(1));
        auto expr1 = std::move(current_expr);
        current_expr = std::make_unique<ExprLangMatches>(std::move(expr0), std::move(expr1));
    }
    else if (ctx->DATATYPE()) {
        visit(ctx->expression(0));
        current_expr = std::make_unique<ExprDatatype>(std::move(current_expr));
    }
    else if (ctx->BOUND()) {
        current_expr = std::make_unique<ExprBound>(Var(ctx->var()->getText().substr(1)));
    }
    else if (ctx->IRI()) {
        visit(ctx->expression(0));
        current_expr = std::make_unique<ExprIRI>(std::move(current_expr));
    }
    else if (ctx->URI()) {
        visit(ctx->expression(0));
        current_expr = std::make_unique<ExprURI>(std::move(current_expr));
    }
    else if (ctx->BNODE()) {
        visit(ctx->expression(0));
        current_expr = std::make_unique<ExprBNode>(std::move(current_expr));
    }
    else if (ctx->RAND()) {
        current_expr = std::make_unique<ExprRand>();
    }
    else if (ctx->ABS()) {
        visit(ctx->expression(0));
        current_expr = std::make_unique<ExprAbs>(std::move(current_expr));
    }
    else if (ctx->CEIL()) {
        visit(ctx->expression(0));
        current_expr = std::make_unique<ExprCeil>(std::move(current_expr));
    }
    else if (ctx->FLOOR()) {
        visit(ctx->expression(0));
        current_expr = std::make_unique<ExprFloor>(std::move(current_expr));
    }
    else if (ctx->ROUND()) {
        visit(ctx->expression(0));
        current_expr = std::make_unique<ExprRound>(std::move(current_expr));
    }
    else if (ctx->CONCAT()) {
        std::vector<std::unique_ptr<Expr>> expr_list;
        if (ctx->expressionList()) {
            for (auto expr_ctx : ctx->expressionList()->expression()) {
                visit(expr_ctx);
                expr_list.push_back(std::move(current_expr));
            }
        }
        current_expr = std::make_unique<ExprConcat>(std::move(expr_list));
    }
    else if (ctx->subStringExpression()) {
        visit(ctx->subStringExpression());
    }
    else if (ctx->STRLEN()) {
        visit(ctx->expression(0));
        current_expr = std::make_unique<ExprStrLen>(std::move(current_expr));
    }
    else if (ctx->strReplaceExpression()) {
        visit(ctx->strReplaceExpression());
    }
    else if (ctx->UCASE()) {
        visit(ctx->expression(0));
        current_expr = std::make_unique<ExprUCase>(std::move(current_expr));
    }
    else if (ctx->LCASE()) {
        visit(ctx->expression(0));
        current_expr = std::make_unique<ExprLCase>(std::move(current_expr));
    }
    else if (ctx->ENCODE_FOR_URI()) {
        visit(ctx->expression(0));
        current_expr = std::make_unique<ExprEncodeForUri>(std::move(current_expr));
    }
    else if (ctx->CONTAINS()) {
        visit(ctx->expression(0));
        auto expr0 = std::move(current_expr);
        visit(ctx->expression(1));
        auto expr1 = std::move(current_expr);
        current_expr = std::make_unique<ExprContains>(std::move(expr0), std::move(expr1));
    }
    else if (ctx->STRSTARTS()) {
        visit(ctx->expression(0));
        auto expr0 = std::move(current_expr);
        visit(ctx->expression(1));
        auto expr1 = std::move(current_expr);
        current_expr = std::make_unique<ExprStrStarts>(std::move(expr0), std::move(expr1));
    }
    else if (ctx->STRENDS()) {
        visit(ctx->expression(0));
        auto expr0 = std::move(current_expr);
        visit(ctx->expression(1));
        auto expr1 = std::move(current_expr);
        current_expr = std::make_unique<ExprStrEnds>(std::move(expr0), std::move(expr1));
    }
    else if (ctx->STRBEFORE()) {
        visit(ctx->expression(0));
        auto expr0 = std::move(current_expr);
        visit(ctx->expression(1));
        auto expr1 = std::move(current_expr);
        current_expr = std::make_unique<ExprStrBefore>(std::move(expr0), std::move(expr1));
    }
    else if (ctx->STRAFTER()) {
        visit(ctx->expression(0));
        auto expr0 = std::move(current_expr);
        visit(ctx->expression(1));
        auto expr1 = std::move(current_expr);
        current_expr = std::make_unique<ExprStrAfter>(std::move(expr0), std::move(expr1));
    }
    else if (ctx->YEAR()) {
        visit(ctx->expression(0));
        current_expr = std::make_unique<ExprYear>(std::move(current_expr));
    }
    else if (ctx->MONTH()) {
        visit(ctx->expression(0));
        current_expr = std::make_unique<ExprMonth>(std::move(current_expr));
    }
    else if (ctx->DAY()) {
        visit(ctx->expression(0));
        current_expr = std::make_unique<ExprDay>(std::move(current_expr));
    }
    else if (ctx->HOURS()) {
        visit(ctx->expression(0));
        current_expr = std::make_unique<ExprHours>(std::move(current_expr));
    }
    else if (ctx->MINUTES()) {
        visit(ctx->expression(0));
        current_expr = std::make_unique<ExprMinutes>(std::move(current_expr));
    }
    else if (ctx->SECONDS()) {
        visit(ctx->expression(0));
        current_expr = std::make_unique<ExprSeconds>(std::move(current_expr));
    }
    else if (ctx->TIMEZONE()) {
        visit(ctx->expression(0));
        current_expr = std::make_unique<ExprTimezone>(std::move(current_expr));
    }
    else if (ctx->TZ()) {
        visit(ctx->expression(0));
        current_expr = std::make_unique<ExprTZ>(std::move(current_expr));
    }
    else if (ctx->NOW()) {
        current_expr = std::make_unique<ExprNow>();
    }
    else if (ctx->UUID()) {
        current_expr = std::make_unique<ExprUUID>();
    }
    else if (ctx->STRUUID()) {
        current_expr = std::make_unique<ExprStrUUID>();
    }
    else if (ctx->MD5()) {
        visit(ctx->expression(0));
        current_expr = std::make_unique<ExprMD5>(std::move(current_expr));
    }
    else if (ctx->SHA1()) {
        visit(ctx->expression(0));
        current_expr = std::make_unique<ExprSHA1>(std::move(current_expr));
    }
    else if (ctx->SHA256()) {
        visit(ctx->expression(0));
        current_expr = std::make_unique<ExprSHA256>(std::move(current_expr));
    }
    else if (ctx->SHA384()) {
        visit(ctx->expression(0));
        current_expr = std::make_unique<ExprSHA384>(std::move(current_expr));
    }
    else if (ctx->SHA512()) {
        visit(ctx->expression(0));
        current_expr = std::make_unique<ExprSHA512>(std::move(current_expr));
    }
    else if (ctx->COALESCE()) {
        std::vector<std::unique_ptr<Expr>> expr_list;
        if (ctx->expressionList()) {
            for (auto expr_ctx : ctx->expressionList()->expression()) {
                visit(expr_ctx);
                expr_list.push_back(std::move(current_expr));
            }
        }
        current_expr = std::make_unique<ExprCoalesce>(std::move(expr_list));
    }
    else if (ctx->IF()) {
        visit(ctx->expression(0));
        auto expr0 = std::move(current_expr);
        visit(ctx->expression(1));
        auto expr1 = std::move(current_expr);
        visit(ctx->expression(2));
        auto expr2 = std::move(current_expr);
        current_expr = std::make_unique<ExprIf>(std::move(expr0),
                                                std::move(expr1),
                                                std::move(expr2));
    }
    else if (ctx->STRLANG()) {
        visit(ctx->expression(0));
        auto expr0 = std::move(current_expr);
        visit(ctx->expression(1));
        auto expr1 = std::move(current_expr);
        current_expr = std::make_unique<ExprStrLang>(std::move(expr0), std::move(expr1));
    }
    else if (ctx->STRDT()) {
        visit(ctx->expression(0));
        auto expr0 = std::move(current_expr);
        visit(ctx->expression(1));
        auto expr1 = std::move(current_expr);
        current_expr = std::make_unique<ExprStrDT>(std::move(expr0), std::move(expr1));
    }
    else if (ctx->SAMETERM()) {
        visit(ctx->expression(0));
        auto expr0 = std::move(current_expr);
        visit(ctx->expression(1));
        auto expr1 = std::move(current_expr);
        current_expr = std::make_unique<ExprSameTerm>(std::move(expr0), std::move(expr1));
    }
    else if (ctx->ISIRI()) {
        visit(ctx->expression(0));
        current_expr = std::make_unique<ExprIsIRI>(std::move(current_expr));
    }
    else if (ctx->ISURI()) {
        visit(ctx->expression(0));
        current_expr = std::make_unique<ExprIsURI>(std::move(current_expr));
    }
    else if (ctx->ISBLANK()) {
        visit(ctx->expression(0));
        current_expr = std::make_unique<ExprIsBlank>(std::move(current_expr));
    }
    else if (ctx->ISLITERAL()) {
        visit(ctx->expression(0));
        current_expr = std::make_unique<ExprIsLiteral>(std::move(current_expr));
    }
    else if (ctx->ISNUMERIC()) {
        visit(ctx->expression(0));
        current_expr = std::make_unique<ExprIsNumeric>(std::move(current_expr));
    }
    else if (ctx->regexExpression()) {
        visit(ctx->regexExpression());
    }
    else if (ctx->existsFunction()) {
        visit(ctx->existsFunction());
    }
    else if (ctx->notExistsFunction()) {
        visit(ctx->notExistsFunction());
    }
    else {
        throw QuerySemanticException("Unhandled built-in call: \"" + ctx->getText() + '"');
    }
    return 0;
}


Any QueryVisitor::visitAggregate(SparqlParser::AggregateContext* ctx) {
    bool distinct = ctx->DISTINCT() != nullptr;
    if (ctx->expression()) {
        // only COUNT(*) doesn't have an expression
        visit(ctx->expression());
    }
    if (ctx->COUNT()) {
        if (ctx->ASTERISK()) {
            current_expr = std::make_unique<ExprAggCountAll>(distinct);
        } else {
            current_expr = std::make_unique<ExprAggCount>(std::move(current_expr), distinct);
        }
    } else if (ctx->SUM()) {
        current_expr = std::make_unique<ExprAggSum>(std::move(current_expr), distinct);
    } else if (ctx->MIN()) {
        current_expr = std::make_unique<ExprAggMin>(std::move(current_expr), distinct);
    } else if (ctx->MAX()) {
        current_expr = std::make_unique<ExprAggMax>(std::move(current_expr), distinct);
    } else if (ctx->AVG()) {
        current_expr = std::make_unique<ExprAggAvg>(std::move(current_expr), distinct);
    } else if (ctx->SAMPLE()) {
        current_expr = std::make_unique<ExprAggSample>(std::move(current_expr), distinct);
    } else if (ctx->GROUP_CONCAT()) {
        std::string separator = " "; // default separator
        if (ctx->SEPARATOR()) {
            separator =  ctx->string()->getText();
            separator = separator.substr(1, separator.size() - 2); // delete quotes
        }
        current_expr = std::make_unique<ExprAggGroupConcat>(std::move(current_expr), separator, distinct);
    } else {
        throw QuerySemanticException("Unhandled aggregate: \"" + ctx->getText() + '"');
    }
    return 0;
}


Any QueryVisitor::visitSubStringExpression(SparqlParser::SubStringExpressionContext* ctx) {
    visit(ctx->expression(0));
    auto expr0 = std::move(current_expr);
    visit(ctx->expression(1));
    auto expr1 = std::move(current_expr);

    if (ctx->expression().size() == 3) {
        visit(ctx->expression(2));
        auto expr2 = std::move(current_expr);
        current_expr = std::make_unique<ExprSubStr>(std::move(expr0),
                                                    std::move(expr1),
                                                    std::move(expr2));
    } else {
        current_expr = std::make_unique<ExprSubStr>(std::move(expr0),
                                                    std::move(expr1));
    }
    return 0;
}


Any QueryVisitor::visitStrReplaceExpression(SparqlParser::StrReplaceExpressionContext* ctx) {
    visit(ctx->expression(0));
    auto expr0 = std::move(current_expr);
    visit(ctx->expression(1));
    auto expr1 = std::move(current_expr);
    visit(ctx->expression(2));
    auto expr2 = std::move(current_expr);

    if (ctx->expression().size() == 4) {
        visit(ctx->expression(3));
        auto expr3 = std::move(current_expr);
        current_expr = std::make_unique<ExprReplace>(std::move(expr0),
                                                     std::move(expr1),
                                                     std::move(expr2),
                                                     std::move(expr3));
    } else {
        current_expr = std::make_unique<ExprReplace>(std::move(expr0),
                                                     std::move(expr1),
                                                     std::move(expr2));
    }
    return 0;
}


Any QueryVisitor::visitRegexExpression(SparqlParser::RegexExpressionContext* ctx) {
    visit(ctx->expression(0));
    auto expr0 = std::move(current_expr);
    visit(ctx->expression(1));
    auto expr1 = std::move(current_expr);

    if (ctx->expression().size() == 3) {
        visit(ctx->expression(2));
        auto expr2 = std::move(current_expr);
        current_expr = std::make_unique<ExprRegex>(std::move(expr0),
                                                   std::move(expr1),
                                                   std::move(expr2));
    } else {
        current_expr = std::make_unique<ExprRegex>(std::move(expr0),
                                                   std::move(expr1));
    }
    return 0;
}


Any QueryVisitor::visitExistsFunction(SparqlParser::ExistsFunctionContext* ctx) {
    visit(ctx->groupGraphPattern());
    assert(current_op != nullptr);
    current_expr = std::make_unique<ExprExists>(std::move(current_op));
    return 0;
}


Any QueryVisitor::visitNotExistsFunction(SparqlParser::NotExistsFunctionContext* ctx) {
    visit(ctx->groupGraphPattern());
    assert(current_op != nullptr);
    current_expr = std::make_unique<ExprNotExists>(std::move(current_op));

    return 0;
}


Any QueryVisitor::visitFunctionCall(SparqlParser::FunctionCallContext*) {
    // TODO: implement this
    throw QuerySemanticException("visitFunctionCall not implemented");
}


Any QueryVisitor::visitTriplesBlock(SparqlParser::TriplesBlockContext* ctx) {
    for (auto& triples_same_subject_path : ctx->triplesSameSubjectPath()) {
        visit(triples_same_subject_path);
    }
    current_op = std::make_unique<OpBasicGraphPattern>(std::move(current_triples), std::move(current_paths));
    return 0;
}


Any QueryVisitor::visitTriplesSameSubjectPath(SparqlParser::TriplesSameSubjectPathContext* ctx) {
    if (ctx->varOrTerm()) {
        // 1. Visit the subject
        visit(ctx->varOrTerm());
        subject_stack.push(std::move(current_sparql_element));
        // 2. Visit the predicate object list
        visit(ctx->propertyListPathNotEmpty());
        subject_stack.pop();
    }
    else {
        // Create new blank node
        SparqlElement new_blank_node(Var("_:b" + std::to_string(blank_node_counter++)));

        subject_stack.push(std::move(new_blank_node));
        // Visit the inner predicates/object lists
        visit(ctx->triplesNodePath());
        // Visit the outer predicate/object list
        visit(ctx->propertyListPath());
        subject_stack.pop();
    }
    return 0;
}


Any QueryVisitor::visitPropertyListPathNotEmpty(SparqlParser::PropertyListPathNotEmptyContext* ctx) {
    // 1. Visit the predicate
    if (ctx->verbPath()) {
        visit(ctx->verbPath());
    }
    else {
        visit(ctx->verbSimple());
    }

    predicate_stack.push(std::move(current_sparql_element));
    // 2. Visit the object list
    auto olp = ctx->objectListPath();
    auto op  = olp->objectPath();
    for (auto& op_item : op) {
        visit(op_item);
    }
    predicate_stack.pop();

    for (auto& property_list_path_not_empty_list : ctx->propertyListPathNotEmptyList()) {
        // 1. Visit the predicate
        if (property_list_path_not_empty_list->verbPath()) {
            visit(property_list_path_not_empty_list->verbPath());
        }
        else {
            visit(property_list_path_not_empty_list->verbSimple());
        }
        predicate_stack.push(std::move(current_sparql_element));

        // 2. Visit the object list
        for (auto& object : property_list_path_not_empty_list->objectList()->object()) {
            visit(object);
        }
        predicate_stack.pop();
    }

    return 0;
}


Any QueryVisitor::visitPropertyListNotEmpty(SparqlParser::PropertyListNotEmptyContext* ctx) {
    for (size_t i = 0; i < ctx->verb().size(); i++) {
        // Visit the predicate
        visit(ctx->verb(i));

        predicate_stack.push(std::move(current_sparql_element));
        for (auto& object : ctx->objectList(i)->object()) {
            visit(object);
        }
        predicate_stack.pop();
    }
    return 0;
}


Any QueryVisitor::visitObjectPath(SparqlParser::ObjectPathContext* ctx) {
    auto gnp = ctx->graphNodePath();
    if (gnp->varOrTerm()) {
        visit(gnp->varOrTerm());

        if (predicate_stack.top().is_path()) {
            current_paths.emplace_back(current_path_variable,
                                       subject_stack.top().duplicate(),
                                       std::move(current_sparql_element),
                                       current_path_semantic,
                                       predicate_stack.top().to_path());
        } else {
            current_triples.emplace_back(subject_stack.top().duplicate(), predicate_stack.top().duplicate(),  std::move(current_sparql_element));
        }
    }
    else {
        SparqlElement new_blank_node(Var("_:b" + std::to_string(blank_node_counter++)));

        subject_stack.push(new_blank_node.duplicate());
        visit(gnp->triplesNodePath());
        subject_stack.pop();

        if (predicate_stack.top().is_path()) {
            current_paths.emplace_back(current_path_variable,
                                       subject_stack.top().duplicate(),
                                       std::move(new_blank_node),
                                       current_path_semantic,
                                       predicate_stack.top().to_path());
        } else {
            current_triples.emplace_back(subject_stack.top().duplicate(), predicate_stack.top().duplicate(), std::move(new_blank_node));
        }
    }
    return 0;
}


Any QueryVisitor::visitObject(SparqlParser::ObjectContext* ctx) {
    auto gn = ctx->graphNode();
    if (gn->varOrTerm()) {
        visit(gn->varOrTerm());

        if (predicate_stack.top().is_path()) {
            current_paths.emplace_back(current_path_variable,
                                       subject_stack.top().duplicate(),
                                       std::move(current_sparql_element),
                                       current_path_semantic,
                                       predicate_stack.top().to_path());
        } else {
            current_triples.emplace_back(subject_stack.top().duplicate(), predicate_stack.top().duplicate(), std::move(current_sparql_element));
        }
    }
    else {
        SparqlElement new_blank_node(Var("_:b" + std::to_string(blank_node_counter++)));

        subject_stack.push(new_blank_node.duplicate());
        visit(gn->triplesNode());
        subject_stack.pop();

        if (predicate_stack.top().is_path()) {
            current_paths.emplace_back(current_path_variable,
                                       subject_stack.top().duplicate(),
                                       std::move(new_blank_node),
                                       current_path_semantic,
                                       predicate_stack.top().to_path());
        } else {
            current_triples.emplace_back(subject_stack.top().duplicate(), predicate_stack.top().duplicate(), std::move(new_blank_node));
        }
    }
    return 0;
}


Any QueryVisitor::visitCollectionPath(SparqlParser::CollectionPathContext*) {
    // TODO: implement this
    throw NotSupportedException("Collection path");
}


Any QueryVisitor::visitCollection(SparqlParser::CollectionContext*) {
    // TODO: implement this
    throw NotSupportedException("Collection");
}


Any QueryVisitor::visitVar(SparqlParser::VarContext* ctx) {
    current_sparql_element = SparqlElement(Var(ctx->getText().substr(1)));
    return 0;
}


Any QueryVisitor::visitIri(SparqlParser::IriContext* ctx) {
    current_sparql_element = SparqlElement(Iri(iriCtxToString(ctx)));
    return 0;
}


Any QueryVisitor::visitRdfLiteral(SparqlParser::RdfLiteralContext* ctx) {
    std::string str = stringCtxToString(ctx->string());
    if (ctx->iri()) {
        std::string iri = iriCtxToString(ctx->iri());
        // xsd:dateTime
        if (iri == "http://www.w3.org/2001/XMLSchema#dateTime") {
            uint64_t datetime_id = DateTime::get_datetime_id(str.c_str());
            if (datetime_id == DateTime::INVALID_ID) {
                throw QueryException("Invalid datetime value: " + str);
            }
            current_sparql_element = SparqlElement(DateTime(datetime_id));
        }
        // xsd:decimal
        else if (iri == "http://www.w3.org/2001/XMLSchema#decimal") {
            current_sparql_element = SparqlElement(Decimal(str));
        }
        // xsd:boolean
        else if (iri == "http://www.w3.org/2001/XMLSchema#boolean") {
            if (str == "true" || str == "1") {
                current_sparql_element = SparqlElement(true);
            } else if (str == "false" || str == "0") {
                current_sparql_element = SparqlElement(false);
            } else {
                throw QueryException("Unsupported boolean value: " + str);
            }
        }
        // TODO: support more datatypes?
        // Unsupported datatypes are interpreted as literals with datatype
        else {
            current_sparql_element = SparqlElement(LiteralDatatype(str, iri));
        }
    }
    else if (ctx->LANGTAG()) {
        current_sparql_element = SparqlElement(LiteralLanguage(str, ctx->LANGTAG()->getText().substr(1)));
    } else {
        current_sparql_element = SparqlElement(Literal(str));
    }
    return 0;
}


Any QueryVisitor::visitNumericLiteralUnsigned(SparqlParser::NumericLiteralUnsignedContext* ctx) {
    current_sparql_element = SparqlElement(Decimal(ctx->getText()));
    return 0;
}


Any QueryVisitor::visitNumericLiteralPositive(SparqlParser::NumericLiteralPositiveContext* ctx) {
    current_sparql_element = SparqlElement(Decimal(ctx->getText()));
    return 0;
}


Any QueryVisitor::visitNumericLiteralNegative(SparqlParser::NumericLiteralNegativeContext* ctx) {
    current_sparql_element = SparqlElement(Decimal(ctx->getText()));
    return 0;
}


Any QueryVisitor::visitBooleanLiteral(SparqlParser::BooleanLiteralContext* ctx) {
    current_sparql_element = SparqlElement(ctx->TRUE() != nullptr);
    return 0;
}


Any QueryVisitor::visitBlankNode(SparqlParser::BlankNodeContext* ctx) {
    if (ctx->BLANK_NODE_LABEL()) {
        current_sparql_element = SparqlElement(Var(ctx->getText()));
    }
    else {
        current_sparql_element = SparqlElement(Var("_:b" + std::to_string(blank_node_counter++)));
    }
    return 0;
}


Any QueryVisitor::visitNil(SparqlParser::NilContext*) {
    current_sparql_element = SparqlElement(Iri("http://www.w3.org/1999/02/22-rdf-syntax-ns#nil"));
    return 0;
}


Any QueryVisitor::visitVerbPath(SparqlParser::VerbPathContext* ctx) {
    // Set current_path
    current_path_inverse = false;
    visit(ctx->path());

    // MillenniumDB's path extension
    if (ctx->AS()) {
         // Path semantic
        if (ctx->ANY_SHORTEST()) {
            current_path_semantic = PathSemantic::ANY_SHORTEST;
        }
        else if (ctx->ALL_SHORTEST()) {
            current_path_semantic = PathSemantic::ALL_SHORTEST;
        }
        else {
            throw QueryException("Unhandled path semantic");
        }
        // Path variable
        current_path_variable = Var(ctx->var()->getText().substr(1));
    }
    // Default SPARQL path
    else {
        // Try to simplify the path
        if (current_path->type() == PathType::PATH_ATOM) {
            // If the path is an Atom
            PathAtom* tmp = dynamic_cast<PathAtom*>(current_path.get());
            if (!tmp->inverse) {
                // And it is not inverted, it can be simplified as an Iri
                current_sparql_element = SparqlElement(Iri(tmp->atom));
                return 0;
            }
        }
        current_path_semantic = PathSemantic::ANY_SHORTEST;
        // This variable name is impossible to be mentioned:
        // 1) As a SPARQL variable (the grammar does not allow it)
        // 2) As a blank node (they start with _:)
        current_path_variable = Var("__" + std::to_string(anonymous_path_counter++));
    }
    current_sparql_element = SparqlElement(std::move(current_path));
    return 0;
}


Any QueryVisitor::visitPathAlternative(SparqlParser::PathAlternativeContext* ctx) {
    if (ctx->pathSequence().size() > 1) {
        std::vector<std::unique_ptr<IPath>> alternatives;
        for (auto& ps_item : ctx->pathSequence()) {
            visit(ps_item);
            alternatives.push_back(std::move(current_path));
        }
        current_path = std::make_unique<PathAlternatives>(std::move(alternatives));
    } else {
        visit(ctx->pathSequence(0));
    }
    return 0;
}


Any QueryVisitor::visitPathSequence(SparqlParser::PathSequenceContext* ctx) {
    if (ctx->pathEltOrInverse().size() > 1) {
        std::vector<std::unique_ptr<IPath>> sequence;
        if (current_path_inverse) {
            for (int i = ctx->pathEltOrInverse().size() - 1; i >= 0; i--) {
                visit(ctx->pathEltOrInverse(i));
                sequence.push_back(std::move(current_path));
            }
        } else {
            for (auto& pe_item : ctx->pathEltOrInverse()) {
                visit(pe_item);
                sequence.push_back(std::move(current_path));
            }
        }
        current_path = std::make_unique<PathSequence>(std::move(sequence));
    } else {
        visit(ctx->pathEltOrInverse(0));
    }
    return 0;
}


Any QueryVisitor::visitPathEltOrInverse(SparqlParser::PathEltOrInverseContext* ctx) {
    auto pe = ctx->pathElt();
    auto pp = pe->pathPrimary();
    auto mod = pe->pathMod();

    bool previous_current_path_inverse = current_path_inverse;
    current_path_inverse = (ctx->INVERSE() != nullptr) ^ current_path_inverse;
    if (pp->path()) {
        visit(pp->path()->pathAlternative());
    } else if (pp->iri()) {
        std::string iri = iriCtxToString(pp->iri());
        current_path = std::make_unique<PathAtom>(std::move(iri), current_path_inverse);
    } else if (pp->A()) {
        current_path = std::make_unique<PathAtom>("http://www.w3.org/1999/02/22-rdf-syntax-ns#type",
                                                  current_path_inverse);
    } else {
        std::vector<PathAtom> negated_set;
        for (auto& path_one : pp->pathNegatedPropertySet()->pathOneInPropertySet()) {
            std::string iri;
            if (path_one->A()) {
                iri = "http://www.w3.org/1999/02/22-rdf-syntax-ns#type";
            } else {
                iri = iriCtxToString(path_one->iri());
            }
            negated_set.push_back(PathAtom(std::move(iri), path_one->INVERSE() != nullptr));
        }
        current_path = std::make_unique<PathNegatedSet>(std::move(negated_set));
    }

    current_path_inverse = previous_current_path_inverse;

    if (mod) {
        switch(mod->getText()[0]) {
            case '*':
                current_path = std::make_unique<PathKleeneStar>(std::move(current_path));
                break;
            case '?':
                if (!current_path->nullable()) {
                    current_path = std::make_unique<PathOptional>(std::move(current_path));
                }
                // else we avoid a redundant PathOptional, current_path stays the same
                break;
            case '+':
                // A+ => A / A*
                auto kleene_star = std::make_unique<PathKleeneStar>(current_path->duplicate());
                std::vector<std::unique_ptr<IPath>> sequence;
                sequence.push_back(std::move(current_path));
                sequence.push_back(std::move(kleene_star));
                current_path = std::make_unique<PathSequence>(std::move(sequence));
                break;
        }
    }
    return 0;
}


Any QueryVisitor::visitVerb(SparqlParser::VerbContext* ctx) {
    if (ctx->A()) {
        current_sparql_element = SparqlElement(Iri("http://www.w3.org/1999/02/22-rdf-syntax-ns#type"));
    } else {
        visit(ctx->varOrIRI());
    }
    return 0;
}


std::string QueryVisitor::iriCtxToString(SparqlParser::IriContext* ctx) {
    std::string iri;
    if (ctx->IRIREF()) {
        iri = ctx->IRIREF()->getText();
        iri = iri.substr(1, iri.size() - 2);
        // Check if the IRI is absolute or not
        // If it is not absolute, it needs to be expanded with the base IRI
        auto pos = iri.find(':');
        if (pos == std::string::npos) {
            if (base_iri.empty()) {
                throw QuerySemanticException("The IRI '" + iri + "' is not absolute and the base IRI is not defined");
            }
            iri = base_iri + iri;
        }
    }
    else {
        std::string prefixedName = ctx->prefixedName()->getText();
        auto pos = prefixedName.find(':');
        auto prefix = prefixedName.substr(0, pos);
        auto suffix = prefixedName.substr(pos + 1);
        if (!prefix_iris_map.contains(prefix)) {
            throw QuerySemanticException("The prefix '" + prefix + "' is not defined");
        }
        iri = prefix_iris_map[prefix] + suffix;
    }
    return iri;
}


std::string QueryVisitor::stringCtxToString(SparqlParser::StringContext* ctx) {
    std::string str = ctx->getText();
    if (ctx->STRING_LITERAL1() || ctx->STRING_LITERAL2()) {
        // One quote per side
        return str.substr(1, str.size() - 2);
    } else {
        // Three quotes per side
        return str.substr(3, str.size() - 6);
    }
}
