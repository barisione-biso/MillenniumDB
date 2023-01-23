#include "binding_id_iter_visitor.h"

#include <iostream>

#include "execution/binding_id_iter/optional_node.h"
#include "execution/binding_id_iter/filter.h"
#include "query_optimizer/plan/join_order/greedy_optimizer.h"
#include "query_optimizer/plan/join_order/leapfrog_optimizer.h"
#include "query_optimizer/rdf_model/expr/expr_to_binding_id_expr.h"
#include "query_optimizer/rdf_model/rdf_model.h"
#include "query_optimizer/rdf_model/plan/triple_plan.h"
#include "query_optimizer/rdf_model/plan/path_plan.h"

using namespace std;
using namespace SPARQL;

BindingIdIterVisitor::BindingIdIterVisitor(ThreadInfo* thread_info, const map<Var, VarId>& var2var_id) :
    var2var_id(var2var_id), thread_info(thread_info) { }


VarId BindingIdIterVisitor::get_var_id(const Var& var) const {
    auto search = var2var_id.find(var);
    if (search != var2var_id.end()) {
        return (*search).second;
    } else {
        throw LogicException("Variable " + var.name + " not present in var_name2var_id");
    }
}


void BindingIdIterVisitor::visit(OpBasicGraphPattern& op_basic_graph_pattern) {
    vector<unique_ptr<Plan>> base_plans;

    for (auto& op_triple : op_basic_graph_pattern.triples) {
        auto subject_id = get_id(op_triple.subject);
        auto predicate_id = get_id(op_triple.predicate);
        auto object_id = get_id(op_triple.object);
        base_plans.push_back(make_unique<TriplePlan>(subject_id, predicate_id, object_id));
    }
    for (auto& op_path : op_basic_graph_pattern.paths) {
        VarId path_var = get_var_id(op_path.var);
        auto subject_id = get_id(op_path.subject);
        auto path = op_path.path.get();
        auto object_id = get_id(op_path.object);
        base_plans.push_back(make_unique<SPARQL::PathPlan>(path_var, subject_id, *path, object_id, op_path.semantic));
    }

    assert(tmp == nullptr);

    // construct var names
    vector<string> var_names;
    const auto binding_size = var2var_id.size();
    var_names.resize(binding_size);
    for (auto&& [var, var_id] : var2var_id) {
        var_names[var_id.id] = var.name;
    }

    // Set input vars
    for (auto& plan : base_plans) {
        plan->set_input_vars(assigned_vars);
    }

    // try to use leapfrog if there is a join
    if (base_plans.size() > 1) {
        tmp = LeapfrogOptimizer::try_get_iter(thread_info, base_plans, var_names, binding_size);
    }

    if (tmp == nullptr) {
        unique_ptr<Plan> root_plan = nullptr;
        root_plan = GreedyOptimizer::get_plan(base_plans, var_names);
        cout << "\nPlan Generated:\n";
        root_plan->print(cout, true, var_names);
        cout << "\nestimated cost: " << root_plan->estimate_cost() << "\n";
        tmp = root_plan->get_binding_id_iter(thread_info);
    }

    // Insert new assigned_vars
    for (auto& plan : base_plans) {
        for (auto var : plan->get_vars()) {
            assigned_vars.insert(var);
        }
    }
}


void BindingIdIterVisitor::visit(OpFilter& op_filter) {
    op_filter.op->accept_visitor(*this);

    vector<std::unique_ptr<BindingIdExpr>> binding_id_exprs;

    Expr2BindingIdExpr expr2binding_id_expr(var2var_id);
    for (auto& expr : op_filter.filters) {
        expr->accept_visitor(expr2binding_id_expr);
        binding_id_exprs.push_back(std::move(expr2binding_id_expr.current_binding_id_expr));
    }

    tmp = make_unique<Filter>(move(tmp), move(binding_id_exprs));
}


void BindingIdIterVisitor::visit(OpOptional&) {
    // TODO:
    // op_optional.op->accept_visitor(*this);
    // unique_ptr<BindingIdIter> binding_id_iter = move(tmp);

    // vector<unique_ptr<BindingIdIter>> optional_children;
    // // TODO: its not necessary to remember assigned_vars and reassign them after visiting a child because we only
    // // support well designed patterns. If we want to support non well designed patterns this could change
    // // auto current_scope_assigned_vars = assigned_vars;

    // for (auto& optional : op_optional.optionals) {
    //     optional->accept_visitor(*this);
    //     optional_children.push_back(move(tmp));
    //     // assigned_vars = current_scope_assigned_vars;
    // }

    // assert(tmp == nullptr);
    // tmp = make_unique<OptionalNode>(move(binding_id_iter), move(optional_children));
}


Id BindingIdIterVisitor::get_id(const SparqlElement& sparql_element) const {
    if (sparql_element.is_var()) {
        return get_var_id(sparql_element.to_var());
    } else {
        return rdf_model.get_object_id(sparql_element);
    }
}
