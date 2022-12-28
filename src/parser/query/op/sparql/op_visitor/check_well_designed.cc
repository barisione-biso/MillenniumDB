#include "check_well_designed.h"

#include "parser/query/op/sparql/ops.h"

using namespace SPARQL;

void CheckWellDesigned::visit(OpOptional& op_optional) {
    // TODO: implement
    // auto local_vars = op_optional.op->get_vars();

    // for (const auto& var : local_vars) {
    //     if (global_vars.find(var) != global_vars.end() && parent_vars.find(var) == parent_vars.end()) {
    //         throw QuerySemanticException("Query is not well defined. Var " + var.name + " is breaking the rule");
    //     }
    //     global_vars.insert(var);
    // }

    // parent_vars = local_vars;
    // for (auto& optional_child : op_optional.optionals) {
    //     optional_child->accept_visitor(*this);
    //     parent_vars = local_vars;
    // }
}


void CheckWellDesigned::visit(OpBasicGraphPattern& op_basic_graph_pattern) {
    auto local_vars = op_basic_graph_pattern.get_vars();

    for (const auto& var : local_vars) {
        if (global_vars.find(var) != global_vars.end() && parent_vars.find(var) == parent_vars.end()) {
            throw QuerySemanticException("Query is not well defined. Var " + var.name + " is breaking the rule");
        }
        global_vars.insert(var);
    }
}


// TODO: OpFilter impose some additional rules
void CheckWellDesigned::visit(OpFilter& op_filter) {
    op_filter.op->accept_visitor(*this);
}

void CheckWellDesigned::visit(OpSelect& op_select) {
    op_select.op->accept_visitor(*this);
}


void CheckWellDesigned::visit(OpWhere& op_where) {
    op_where.op->accept_visitor(*this);
}



void CheckWellDesigned::visit(OpOrderBy& op_order_by) {
    op_order_by.op->accept_visitor(*this);
}
