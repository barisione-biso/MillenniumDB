#include "check_var_names.h"

#include "base/parser/logical_plan/exceptions.h"
#include "base/parser/logical_plan/op/op_filter.h"
#include "base/parser/logical_plan/op/op_node_label.h"
#include "base/parser/logical_plan/op/op_edge_label.h"
#include "base/parser/logical_plan/op/op_node_property.h"
#include "base/parser/logical_plan/op/op_edge_property.h"
#include "base/parser/logical_plan/op/op_select.h"
#include "base/parser/logical_plan/op/op_match.h"
#include "base/parser/logical_plan/op/op_connection.h"
#include "base/parser/logical_plan/op/op_lonely_node.h"
#include "base/parser/logical_plan/op/op_node_loop.h"

void CheckVarNames::visit(OpSelect& op_select) {
    op_select.op->accept_visitor(*this);

    for (auto& select_item : op_select.select_items) {
        auto node_search = node_names.find(select_item.first);
        if (node_search == node_names.end()) {
            auto edge_search = edge_names.find(select_item.first);
            if (edge_search == edge_names.end()) {
                throw QuerySemanticException("Variable \"" + select_item.first +
                    "\" used in SELECT is not declared in MATCH");
            }
        }
    }
}


void CheckVarNames::visit(OpMatch& op_match) {
    node_names = op_match.node_names;
    edge_names = op_match.edge_names;
}


void CheckVarNames::visit(OpFilter& op_filter) {
    op_filter.op->accept_visitor(*this);
    if (op_filter.condition != nullptr)
        op_filter.condition->check_names(node_names, edge_names);
}


void CheckVarNames::visit(OpNodeLabel&) { };
void CheckVarNames::visit(OpEdgeLabel&) { };
void CheckVarNames::visit(OpNodeProperty&) { };
void CheckVarNames::visit(OpEdgeProperty&) { };
void CheckVarNames::visit(OpConnection&) { };
void CheckVarNames::visit(OpLonelyNode&) { };
void CheckVarNames::visit(OpNodeLoop&) { };
