#include "check_var_names.h"

#include "base/parser/logical_plan/exceptions.h"
#include "base/parser/logical_plan/op/op_distinct.h"
#include "base/parser/logical_plan/op/op_filter.h"
#include "base/parser/logical_plan/op/op_graph_pattern_root.h"
#include "base/parser/logical_plan/op/op_group_by.h"
#include "base/parser/logical_plan/op/op_basic_graph_pattern.h"
#include "base/parser/logical_plan/op/op_optional.h"
#include "base/parser/logical_plan/op/op_order_by.h"
#include "base/parser/logical_plan/op/op_select.h"

void CheckVarNames::visit(OpSelect& op_select) {
    op_select.op->accept_visitor(*this);

    for (auto& select_item : op_select.select_items) {
        if (declared_object_names.find(select_item.var) == declared_object_names.end()) {
            throw QuerySemanticException("Variable \"" + select_item.var +
                "\" used in SELECT is not declared in MATCH");
        }
    }
}


void CheckVarNames::visit(OpOptional& op_optional) {
    op_optional.op->accept_visitor(*this);
    for (auto& optional_child : op_optional.optionals) {
        optional_child->accept_visitor(*this);
    }
}


void CheckVarNames::visit(OpBasicGraphPattern& op_basic_graph_pattern) {
    for (const auto& var : op_basic_graph_pattern.vars) {
        declared_object_names.insert(var.name);
    }
}


void CheckVarNames::visit(OpFilter& op_filter) {
    op_filter.op->accept_visitor(*this);
    op_filter.check_var_names(declared_object_names);
}


void CheckVarNames::visit(OpGroupBy& op_group_by) {
    op_group_by.op->accept_visitor(*this);
}


void CheckVarNames::visit(OpOrderBy& op_order_by) {
    op_order_by.op->accept_visitor(*this);
}


void CheckVarNames::visit(OpGraphPatternRoot& op_graph_pattern_root) {
     op_graph_pattern_root.op->accept_visitor(*this);
}


void CheckVarNames::visit(OpDistinct& op_distinct) {
    op_distinct.op->accept_visitor(*this);
}
