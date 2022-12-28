#include "check_scoped_blank_nodes.h"

#include "base/exceptions.h"
#include "parser/query/op/sparql/ops.h"

using namespace SPARQL;

void CheckScopedBlankNodes::visit(OpOptional& op_optional) {
    // Visit op_triples
    op_optional.lhs->accept_visitor(*this);
    op_optional.rhs->accept_visitor(*this);
}


void CheckScopedBlankNodes::visit(OpBasicGraphPattern& op_basic_graph_pattern) {
    // It is important to notice that we treat blank nodes as
    // "new variables that can't be referred to"
    // and we identify them by their name (they start with "_:")
    std::set<Var> scope_blank_nodes;
    for (auto& triple : op_basic_graph_pattern.triples) {
        if (triple.subject.is_bnode()) {
            scope_blank_nodes.insert(triple.subject.to_var());
        }
        if (triple.object.is_bnode()) {
            scope_blank_nodes.insert(triple.object.to_var());
        }
    }
    for (auto& path : op_basic_graph_pattern.paths) {
        if (path.subject.is_bnode()) {
            scope_blank_nodes.insert(path.subject.to_var());
        }
        if (path.object.is_bnode()) {
            scope_blank_nodes.insert(path.object.to_var());
        }
    }
    // Check if any blank node was mentioned before in another scope
    for (auto& bnode : scope_blank_nodes) {
        if (mentioned_blank_nodes.find(bnode) != mentioned_blank_nodes.end()) {
            throw QuerySemanticException("Query is not well defined. Blank node "
                                         + bnode.name
                                         + " is breaking the rule");
        }
    }
    // Store current scope blank nodes into mentioned blank nodes
    mentioned_blank_nodes.insert(scope_blank_nodes.begin(), scope_blank_nodes.end());
}


void CheckScopedBlankNodes::visit(OpFilter& op_filter) {
    op_filter.op->accept_visitor(*this);
}


void CheckScopedBlankNodes::visit(OpSelect& op_select) {
    op_select.op->accept_visitor(*this);
}


void CheckScopedBlankNodes::visit(OpWhere& op_where) {
    op_where.op->accept_visitor(*this);
}


void CheckScopedBlankNodes::visit(OpOrderBy& op_order_by) {
    op_order_by.op->accept_visitor(*this);
}
