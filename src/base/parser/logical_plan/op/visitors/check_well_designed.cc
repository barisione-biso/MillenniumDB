#include "check_well_designed.h"

#include <iostream>

#include "base/parser/logical_plan/exceptions.h"
#include "base/parser/logical_plan/op/op_distinct.h"
#include "base/parser/logical_plan/op/op_filter.h"
#include "base/parser/logical_plan/op/op_graph_pattern_root.h"
#include "base/parser/logical_plan/op/op_group_by.h"
#include "base/parser/logical_plan/op/op_match.h"
#include "base/parser/logical_plan/op/op_optional.h"
#include "base/parser/logical_plan/op/op_order_by.h"
#include "base/parser/logical_plan/op/op_select.h"
#include "base/parser/logical_plan/op/op_path.h"
#include "base/parser/logical_plan/op/op_path_alternatives.h"
#include "base/parser/logical_plan/op/op_path_atom.h"
#include "base/parser/logical_plan/op/op_path_sequence.h"
#include "base/parser/logical_plan/op/op_path_kleene_star.h"
#include "base/parser/logical_plan/op/op_path_optional.h"



using namespace std;

void CheckWellDesigned::visit(OpOptional& op_optional) {
    auto local_vars = op_optional.op->get_var_names();

    for (const auto var : local_vars) {
        if (global.find(var) != global.end() && parent.find(var) == parent.end()) {
            throw QuerySemanticException("Query is not well defined. Var " + var + " is breaking the rule");
        }
        global.insert(var);
    }

    parent = local_vars;
    for (auto& optional_child : op_optional.optionals) {
        optional_child->accept_visitor(*this);
        parent = local_vars;
    }
}


void CheckWellDesigned::visit(OpMatch& op_match) {
    auto local_vars = op_match.get_var_names();

    for (const auto var : local_vars) {
        if (global.find(var) != global.end() && parent.find(var) == parent.end()) {
            throw QuerySemanticException("Query is not well defined. Var " + var + " is breaking the rule");
        }
        global.insert(var);
    }
}


void CheckWellDesigned::visit(OpGraphPatternRoot& op_graph_pattern_root) {
    op_graph_pattern_root.op->accept_visitor(*this);
}


void CheckWellDesigned::visit(OpSelect& op_select) {
    op_select.op->accept_visitor(*this);
}


void CheckWellDesigned::visit(OpFilter& op_filter) {
    op_filter.op->accept_visitor(*this);

}


void CheckWellDesigned::visit(OpGroupBy& op_group_by) {
    op_group_by.op->accept_visitor(*this);

}


void CheckWellDesigned::visit(OpOrderBy& op_order_by) {
    op_order_by.op->accept_visitor(*this);
}


void CheckWellDesigned::visit(OpDistinct& op_distinct) {
    op_distinct.op->accept_visitor(*this);
}


void CheckWellDesigned::visit(OpConnection&)        { }
void CheckWellDesigned::visit(OpConnectionType&)    { }
void CheckWellDesigned::visit(OpLabel&)             { }
void CheckWellDesigned::visit(OpProperty&)          { }
void CheckWellDesigned::visit(OpUnjointObject&)     { }

void CheckWellDesigned::visit(OpPropertyPath&)      { }
void CheckWellDesigned::visit(OpPath&)              { }
void CheckWellDesigned::visit(OpPathAlternatives&)  { }
void CheckWellDesigned::visit(OpPathSequence&)      { }
void CheckWellDesigned::visit(OpPathAtom&)          { }
void CheckWellDesigned::visit(OpPathKleeneStar&)     { }
void CheckWellDesigned::visit(OpPathOptional&)       { }
