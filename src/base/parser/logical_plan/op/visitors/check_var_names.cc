#include "check_var_names.h"

#include "base/parser/logical_plan/exceptions.h"
#include "base/parser/logical_plan/op/op_connection.h"
#include "base/parser/logical_plan/op/op_filter.h"
#include "base/parser/logical_plan/op/op_graph_pattern_root.h"
#include "base/parser/logical_plan/op/op_group_by.h"
#include "base/parser/logical_plan/op/op_match.h"
#include "base/parser/logical_plan/op/op_optional.h"
#include "base/parser/logical_plan/op/op_order_by.h"
#include "base/parser/logical_plan/op/op_order_by.h"
#include "base/parser/logical_plan/op/op_select.h"

void CheckVarNames::visit(const OpSelect& op_select) {
    op_select.op->accept_visitor(*this);

    for (auto& select_item : op_select.select_items) {
        if (declared_object_names.find(select_item.var) == declared_object_names.end()) {
            throw QuerySemanticException("Variable \"" + select_item.var +
                "\" used in SELECT is not declared in MATCH");
        }
    }
}


void CheckVarNames::visit(const OpOptional& op_optional) {
    op_optional.op->accept_visitor(*this);
    for (auto& optional_child : op_optional.optionals) {
        optional_child->accept_visitor(*this);
    }
}


void CheckVarNames::visit(const OpMatch& op_match) {
    auto var_names = move(op_match.var_names);
    for (auto& var_name : var_names) {
        declared_object_names.insert(var_name);
    }
}


void CheckVarNames::visit(const OpFilter& op_filter) {
    op_filter.op->accept_visitor(*this);
    op_filter.check_var_names(declared_object_names);
}


void CheckVarNames::visit(const OpGroupBy& op_group_by) {
    op_group_by.op->accept_visitor(*this);
}


void CheckVarNames::visit(const OpOrderBy& op_order_by) {
    op_order_by.op->accept_visitor(*this);
}


void CheckVarNames::visit(const OpGraphPatternRoot& op_graph_pattern_root) {
     op_graph_pattern_root.op->accept_visitor(*this);
}


void CheckVarNames::visit(const OpTransitiveClosure&) { }
void CheckVarNames::visit(const OpConnection&) { }
void CheckVarNames::visit(const OpConnectionType&) { }
void CheckVarNames::visit(const OpLabel&) { }
void CheckVarNames::visit(const OpProperty&) { }
void CheckVarNames::visit(const OpUnjointObject&) { }
