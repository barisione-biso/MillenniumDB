#include "check_var_names.h"

#include "base/parser/logical_plan/exceptions.h"
#include "base/parser/logical_plan/op/op_filter.h"
#include "base/parser/logical_plan/op/op_select.h"
#include "base/parser/logical_plan/op/op_match.h"
#include "base/parser/logical_plan/op/op_connection.h"

void CheckVarNames::visit(const OpSelect& op_select) {
    op_select.op->accept_visitor(*this);

    for (auto& select_item : op_select.select_items) {
        if (declared_object_names.find(select_item.var) == declared_object_names.end()) {
            throw QuerySemanticException("Variable \"" + select_item.var +
                "\" used in SELECT is not declared in MATCH");
        }
    }
}


void CheckVarNames::visit(const OpMatch& op_match) {
    declared_object_names = op_match.var_names;
}


void CheckVarNames::visit(const OpFilter& op_filter) {
    op_filter.op->accept_visitor(*this);
    op_filter.check_var_names(declared_object_names);
}


void CheckVarNames::visit(const OpGroupBy&) { }
void CheckVarNames::visit(const OpOrderBy&) { }
void CheckVarNames::visit(const OpTransitiveClosure&) { }
void CheckVarNames::visit(const OpConnection&) { }
void CheckVarNames::visit(const OpConnectionType&) { }
void CheckVarNames::visit(const OpLabel&) { }
void CheckVarNames::visit(const OpProperty&) { }
void CheckVarNames::visit(const OpUnjointObject&) { }
