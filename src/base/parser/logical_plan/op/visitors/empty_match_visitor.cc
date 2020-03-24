#include "empty_match_visitor.h"

#include "base/parser/logical_plan/op/op_select.h"
#include "base/parser/logical_plan/op/op_match.h"
#include "base/parser/logical_plan/op/op_filter.h"
#include "base/parser/logical_plan/op/op_label.h"
#include "base/parser/logical_plan/op/op_property.h"
#include "base/parser/logical_plan/op/op_connection.h"

void EmptyMatchVisitor::visit(OpSelect& op_select) {
    select_all = op_select.select_all;
    op_select.op->accept_visitor(*this);
}


void EmptyMatchVisitor::visit(OpMatch& op_match) {
    if (select_all
        && op_match.labels.size() == 0
        && op_match.properties.size() == 0
        && op_match.connections.size() == 0)
    {
        throw NotSupportedException("Empty MATCH with SELECT *");
    }
}


void EmptyMatchVisitor::visit(OpFilter& op_filter) {
    op_filter.op->accept_visitor(*this);
}


void EmptyMatchVisitor::visit(OpLabel&) { }
void EmptyMatchVisitor::visit(OpProperty&) { }
void EmptyMatchVisitor::visit(OpConnection&) { }
