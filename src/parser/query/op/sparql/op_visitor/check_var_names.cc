#include "check_var_names.h"

#include <set>

#include "base/exceptions.h"
#include "parser/query/op/sparql/ops.h"

using namespace SPARQL;

void CheckVarNames::visit(OpSelect& op_select) {
    op_select.op->accept_visitor(*this);
    std::set<Var> where_vars = op_select.get_vars();
    for (auto& var : op_select.vars) {
        if (where_vars.find(var) == where_vars.end()) {
            throw QuerySemanticException("Variable \"" + var.name + "\" is not declared");
        }
    }
}


void CheckVarNames::visit(OpBasicGraphPattern& op_basic_graph_pattern) {
    for (auto& op_triple : op_basic_graph_pattern.triples) {
        for (auto& var : op_triple.get_vars()) {
            declared_vars.insert(var);
            if (declared_path_vars.find(var) != declared_path_vars.end()) {
                throw QuerySemanticException("Duplicated path variable \"" + var.name
                                             + "\". Paths must have an unique variable");
            }
        }
    }

    for (auto& op_path : op_basic_graph_pattern.paths) {
        if (op_path.subject.is_var()) {
            auto var = op_path.subject.to_var();
            declared_vars.insert(var);
            if (declared_path_vars.find(var) != declared_path_vars.end()) {
                throw QuerySemanticException("Duplicated path variable \"" + var.name
                                             + "\". Paths must have an unique variable");
            }
        }

        if (op_path.object.is_var()) {
            auto var = op_path.object.to_var();
            declared_vars.insert(var);
            if (declared_path_vars.find(var) != declared_path_vars.end()) {
                throw QuerySemanticException("Duplicated path variable \"" + var.name
                                             + "\". Paths must have an unique variable");
            }
        }

        if (!declared_path_vars.insert(op_path.var).second) {
            throw QuerySemanticException("Duplicated path variable \"" + op_path.var.name
                                         + "\". Paths must have an unique variable");
        }
        if (!declared_vars.insert(op_path.var).second) {
            throw QuerySemanticException("Duplicated path variable \"" + op_path.var.name
                                         + "\". Paths must have an unique variable");
        }
    }
}


void CheckVarNames::visit(OpFilter& op_filter) {
    op_filter.op->accept_visitor(*this);

    for (const auto& expr : op_filter.filters) {
        auto expr_vars = expr->get_vars();
        for (const auto& expr_var : expr_vars) {
            if (declared_vars.find(expr_var) == declared_vars.end()) {
                throw QuerySemanticException("Variable \"" + expr_var.name + "\" is not declared");
            }
        }
    }
}


void CheckVarNames::visit(OpWhere& op_where) {
    op_where.op->accept_visitor(*this);
}


void CheckVarNames::visit(OpOptional& op_optional) {
    op_optional.lhs->accept_visitor(*this);
    op_optional.rhs->accept_visitor(*this);
}


void CheckVarNames::visit(OpOrderBy& op_order_by) {
    op_order_by.op->accept_visitor(*this);
    // TODO: implement when OpOrderBy has its final form
}
