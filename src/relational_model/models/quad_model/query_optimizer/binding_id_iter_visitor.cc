#include "binding_id_iter_visitor.h"

#include <cassert>
#include <iostream>

#include "base/parser/logical_plan/op/op_basic_graph_pattern.h"
#include "base/parser/logical_plan/op/op_optional.h"
#include "base/parser/logical_plan/op/op_path.h"
#include "base/parser/logical_plan/op/op_path_alternatives.h"
#include "base/parser/logical_plan/op/op_path_atom.h"
#include "base/parser/logical_plan/op/op_path_sequence.h"
#include "base/parser/logical_plan/op/op_path_kleene_star.h"
#include "base/parser/logical_plan/op/op_path_optional.h"
#include "relational_model/execution/binding_id_iter/optional_node.h"
#include "relational_model/execution/binding_id_iter/leapfrog_join.h"
#include "relational_model/execution/binding_id_iter/empty_binding_id_iter.h"
#include "relational_model/execution/binding_id_iter/single_result_binding_id_iter.h"
#include "relational_model/models/quad_model/query_optimizer/join_plan/connection_plan.h"
#include "relational_model/models/quad_model/query_optimizer/join_plan/label_plan.h"
#include "relational_model/models/quad_model/query_optimizer/join_plan/nested_loop_plan.h"
#include "relational_model/models/quad_model/query_optimizer/join_plan/property_plan.h"
#include "relational_model/models/quad_model/query_optimizer/join_plan/property_path_plan.h"
#include "relational_model/models/quad_model/query_optimizer/join_plan/unjoint_object_plan.h"
#include "relational_model/models/quad_model/query_optimizer/join_plan/hash_join_grace_plan.h"
#include "relational_model/models/quad_model/query_optimizer/join_plan/hash_join_in_memory_plan.h"
#include "relational_model/models/quad_model/query_optimizer/join_plan/hash_join_in_buffer_plan.h"
#include "relational_model/models/quad_model/query_optimizer/selinger_optimizer.h"
#include "storage/index/bplus_tree/leapfrog_iter.h"

using namespace std;

constexpr auto MAX_SELINGER_PLANS = 0;

BindingIdIterVisitor::BindingIdIterVisitor(const QuadModel& model,
                                           const map<Var, VarId>& var2var_id,
                                           ThreadInfo* thread_info) :
    model       (model),
    var2var_id  (var2var_id),
    thread_info (thread_info) { }


VarId BindingIdIterVisitor::get_var_id(const Var& var) {
    auto search = var2var_id.find(var);
    if (search != var2var_id.end()) {
        return (*search).second;
    } else {
        throw std::logic_error("variable " + var.name + " not present in var_name2var_id");
    }
}


void BindingIdIterVisitor::visit(OpBasicGraphPattern& op_basic_graph_pattern) {
    // Process Isolated Terms
    // if a term is not found we can asume the MATCH result is empty
    for (auto& isolated_term : op_basic_graph_pattern.isolated_terms) {
        ObjectId term = model.get_object_id(isolated_term.term.to_graph_object());
        if (term.is_not_found()) {
            tmp = make_unique<EmptyBindingIdIter>();
            return;
        } else if ((term.id & GraphModel::TYPE_MASK) == GraphModel::ANONYMOUS_NODE_MASK) {
            auto anon_id = term.id & GraphModel::VALUE_MASK;
            if (anon_id > model.catalog().anonymous_nodes_count) {
                tmp = make_unique<EmptyBindingIdIter>();
                return;
            } else {
                tmp = make_unique<SingleResultBindingIdIter>();
            }
        } else if ((term.id & GraphModel::TYPE_MASK) == GraphModel::CONNECTION_MASK) {
            auto conn_id = term.id & GraphModel::VALUE_MASK;
            if (conn_id > model.catalog().connections_count) {
                tmp = make_unique<EmptyBindingIdIter>();
                return;
            } else {
                tmp = make_unique<SingleResultBindingIdIter>();
            }
        } else {
            // search in nodes
            auto r = RecordFactory::get(term.id);
            bool interruption_requested = false;
            auto it = model.nodes->get_range(&interruption_requested, r, r);
            if (it->next() == nullptr) {
                tmp = make_unique<EmptyBindingIdIter>();
                return;
            }
        }
    }
    vector<unique_ptr<JoinPlan>> base_plans;

    // Process Isolated Vars
    for (auto& isolated_var : op_basic_graph_pattern.isolated_vars) {
        base_plans.push_back(
            make_unique<UnjointObjectPlan>(model, get_var_id(isolated_var.var))
        );
    }

    // Process Labels
    for (auto& op_label : op_basic_graph_pattern.labels) {
        auto label_id = model.get_object_id(GraphObject::make_string(op_label.label));

        if (op_label.node_id.is_var()) {
            auto node_var_id = get_var_id(op_label.node_id.to_var());
            base_plans.push_back(
                make_unique<LabelPlan>(model, node_var_id, label_id)
            );
        } else {
            auto node_id = model.get_object_id(op_label.node_id.to_graph_object());
            base_plans.push_back(
                make_unique<LabelPlan>(model, node_id, label_id)
            );
        }
    }

    // Process properties from Match
    for (auto& op_property : op_basic_graph_pattern.properties) {
        auto key_id   = model.get_object_id(GraphObject::make_string(op_property.key));
        auto value_id = get_value_id(op_property.value);

        if (op_property.node_id.is_var()) {
            auto obj_var_id = get_var_id(op_property.node_id.to_var());

            base_plans.push_back(
                make_unique<PropertyPlan>(model, obj_var_id, key_id, value_id)
            );
        } else {
            auto obj_id = model.get_object_id(op_property.node_id.to_graph_object());
            base_plans.push_back(
                make_unique<PropertyPlan>(model, obj_id, key_id, value_id)
            );
        }
    }

    // Process connections
    for (auto& op_connection : op_basic_graph_pattern.connections) {
        auto from_id = op_connection.from.is_var()
                        ? (JoinPlan::Id) get_var_id(op_connection.from.to_var())
                        : (JoinPlan::Id) model.get_object_id(op_connection.from.to_graph_object());

        auto to_id   = op_connection.to.is_var()
                        ? (JoinPlan::Id) get_var_id(op_connection.to.to_var())
                        : (JoinPlan::Id) model.get_object_id(op_connection.to.to_graph_object());

        auto edge_id = op_connection.edge.is_var()
                        ? (JoinPlan::Id) get_var_id(op_connection.edge.to_var())
                        : (JoinPlan::Id) model.get_object_id(op_connection.edge.to_graph_object());

        if (op_connection.types.empty()) {
            // Type not mentioned, creating anonymous variable for type
            // Important: this name must be consistent with generated at OpBasicGraphPattern
            auto tmp_str = op_connection.edge.to_string();
            if (tmp_str[0] == '?') {
                tmp_str.erase(0, 1);
            }
            auto type_var_id = get_var_id(Var("?_typeof_" + tmp_str));
            base_plans.push_back(
                make_unique<ConnectionPlan>(model, from_id, to_id, type_var_id, edge_id));
        }
        else if (op_connection.types.size() == 1) {
            if (op_connection.types[0][0] == '?') {
                // Type is an explicit variable
                auto type_var_id = get_var_id(Var(op_connection.types[0]));
                base_plans.push_back(
                    make_unique<ConnectionPlan>(model, from_id, to_id, type_var_id, edge_id));
            } else {
                // Type is an IdentifiebleNode
                auto type_obj_id = model.get_object_id(GraphObject::make_identifiable(op_connection.types[0]));
                base_plans.push_back(
                    make_unique<ConnectionPlan>(model, from_id, to_id, type_obj_id, edge_id)
                );
            }
        }
        else {
            throw QuerySemanticException("Connections can't have multiple types when using QuadModel");
        }
    }

    for (auto& property_path : op_basic_graph_pattern.property_paths) {
        auto from_id = property_path.from.is_var()
                    ? (JoinPlan::Id) get_var_id(property_path.from.to_var())
                    : (JoinPlan::Id) model.get_object_id(property_path.from.to_graph_object());

        auto to_id   = property_path.to.is_var()
                    ? (JoinPlan::Id) get_var_id(property_path.to.to_var())
                    : (JoinPlan::Id) model.get_object_id(property_path.to.to_graph_object());


        VarId path_var = get_var_id(property_path.var);
        base_plans.push_back(
            make_unique<PropertyPathPlan>(model, path_var, from_id, to_id, *property_path.path)
        );
    }

    assert(tmp == nullptr);

    // construct var names
    vector<string> var_names;
    const auto binding_size = var2var_id.size();
    var_names.resize(binding_size);
    for (auto&& [var, var_id] : var2var_id) {
        var_names[var_id.id] = var.name;
    }

    // construct input vars
    uint64_t input_vars = 0;
    assert(assigned_vars.size() <= 64 && "for now, a maximum of 64 variables is supported");
    for (auto var_id : assigned_vars) {
        input_vars |= 1UL << var_id.id;
    }

    unique_ptr<JoinPlan> root_plan = nullptr;
    if (base_plans.size() == 0) {
        tmp = make_unique<SingleResultBindingIdIter>();
        return;
    }

    // try to use leapfrog if the is a join
    if (base_plans.size() > 1) {
        tmp = try_get_leapfrog_plan(base_plans, var_names, binding_size, input_vars);
    }

    if (tmp == nullptr) {
        if (base_plans.size() <= MAX_SELINGER_PLANS) {
            SelingerOptimizer selinger_optimizer(move(base_plans), var_names, input_vars);
            root_plan = selinger_optimizer.get_plan();
        } else {
            root_plan = get_greedy_join_plan(move(base_plans), var_names, input_vars);
        }

        // insert new assigned_vars
        const auto new_assigned_vars = root_plan->get_vars();
        for (std::size_t i = 0; i < binding_size; i++) {
            if ((new_assigned_vars & (1UL << i)) != 0) {
                assigned_vars.emplace(i);
            }
        }

        std::cout << "\nPlan Generated:\n";
        root_plan->print(2, true, var_names);
        std::cout << "\nestimated cost: " << root_plan->estimate_cost() << "\n";

        tmp = root_plan->get_binding_id_iter(thread_info);
    }
}


void BindingIdIterVisitor::visit(OpOptional& op_optional) {
    op_optional.op->accept_visitor(*this);
    unique_ptr<BindingIdIter> binding_id_iter = move(tmp);

    vector<unique_ptr<BindingIdIter>> optional_children;
    // TODO: its not necessary to remember assigned_vars and reassign them after visiting a child because we only
    // support well designed patterns. If we want to support non well designed patterns this could change
    // auto current_scope_assigned_vars = assigned_vars;

    for (auto& optional : op_optional.optionals) {
        optional->accept_visitor(*this);
        optional_children.push_back(move(tmp));
        // assigned_vars = current_scope_assigned_vars;
    }

    assert(tmp == nullptr);
    tmp = make_unique<OptionalNode>(move(binding_id_iter), move(optional_children));
}


unique_ptr<JoinPlan> BindingIdIterVisitor::get_greedy_join_plan(
    vector<unique_ptr<JoinPlan>> base_plans,
    vector<string>& var_names,
    uint64_t input_vars)
{
    const auto base_plans_size = base_plans.size();
    assert(base_plans_size > 0);

    // choose the first scan
    int best_index = 0;
    double best_cost = std::numeric_limits<double>::max();
    for (size_t j = 0; j < base_plans_size; j++) {
        base_plans[j]->set_input_vars(input_vars);
        auto current_element_cost = base_plans[j]->estimate_cost();
        base_plans[j]->print(0, true, var_names);
        std::cout << "\n";
        if (current_element_cost < best_cost) {
            best_cost = current_element_cost;
            best_index = j;
        }
    }
    auto root_plan = move(base_plans[best_index]);

    // choose the next scan and make a IndexNestedLoopJoin
    for (size_t i = 1; i < base_plans_size; i++) {
        best_index = 0;
        best_cost = std::numeric_limits<double>::max();
        unique_ptr<JoinPlan> best_step_plan = nullptr;

        for (size_t j = 0; j < base_plans_size; j++) {
            if (base_plans[j] != nullptr
                && !base_plans[j]->cartesian_product_needed(*root_plan) )
            {
                auto nested_loop_plan = make_unique<NestedLoopPlan>(root_plan->duplicate(), base_plans[j]->duplicate());
                auto nested_loop_cost = nested_loop_plan->estimate_cost();

                if (nested_loop_cost < best_cost) {
                    best_cost = nested_loop_cost;
                    best_index = j;
                    best_step_plan = move(nested_loop_plan);
                }
                // <HashJoinGracePlan>, <HashJoinInMemoryPlan>, <HashJoinInBufferPlan>
                // auto hash_join_plan = make_unique<HashJoinInBufferPlan>(root_plan->duplicate(), base_plans[j]->duplicate());
                // auto hash_join_cost = hash_join_plan->estimate_cost();

                // if (hash_join_cost < best_cost) {
                //     best_cost = hash_join_cost;
                //     best_index = j;
                //     best_step_plan = move(hash_join_plan);
                // }
            }
        }

        // All elements would form a cross product, iterate again, allowing cross products
        if (best_cost == std::numeric_limits<double>::max()) {
            best_index = 0;

            for (size_t j = 0; j < base_plans_size; j++) {
                if (base_plans[j] == nullptr) {
                    continue;
                }
                auto nested_loop_plan = make_unique<NestedLoopPlan>(root_plan->duplicate(), base_plans[j]->duplicate());
                auto nested_loop_cost = nested_loop_plan->estimate_cost();

                if (nested_loop_cost < best_cost) {
                    best_cost = nested_loop_cost;
                    best_index = j;
                    best_step_plan = move(nested_loop_plan);
                }
            }
        }
        base_plans[best_index] = nullptr;
        root_plan = move(best_step_plan);
    }

    return root_plan;
}


unique_ptr<BindingIdIter> BindingIdIterVisitor::try_get_leapfrog_plan(const vector<unique_ptr<JoinPlan>>& base_plans,
                                                                      vector<string>& var_names,
                                                                      const size_t binding_size,
                                                                      uint64_t input_vars)
{
    const auto base_plans_size = base_plans.size();
    assert(base_plans_size > 0);
    vector<unique_ptr<JoinPlan>> ordered_plans;
    vector<bool> base_plan_selected(base_plans_size, false);

    vector<VarId> intersection_vars;
    set<VarId> enumeration_vars;
    set<VarId> assigned_vars_set(assigned_vars.begin(), assigned_vars.end());
    set<VarId> intersection_vars_set;

    // choose the first plan
    int best_index = 0;
    double best_cost = std::numeric_limits<double>::max();
    for (size_t j = 0; j < base_plans_size; j++) {
        base_plans[j]->set_input_vars(input_vars);
        auto current_element_cost = base_plans[j]->estimate_cost();
        if (current_element_cost < best_cost) {
            best_cost = current_element_cost;
            best_index = j;
        }
    }
    base_plan_selected[best_index] = true;
    // cout << "selected index " << best_index << "\n";
    ordered_plans.push_back(base_plans[best_index]->duplicate());

    // choose the next plan
    for (size_t i = 1; i < base_plans_size; i++) {
        best_index = 0;
        best_cost = std::numeric_limits<double>::max();
        unique_ptr<JoinPlan> best_step_plan = nullptr;

        for (size_t j = 0; j < base_plans_size; j++) {
            if (!base_plan_selected[j]
                && !base_plans[j]->cartesian_product_needed(input_vars) )
            {
                auto duplicated_plan = base_plans[j]->duplicate();
                duplicated_plan->set_input_vars(input_vars);
                auto cost = duplicated_plan->estimate_cost();

                if (cost < best_cost) {
                    best_cost = cost;
                    best_index = j;
                    best_step_plan = move(duplicated_plan);
                }
            }
        }

        // All elements would form a cross product, iterate again, allowing cross products
        if (best_cost == std::numeric_limits<double>::max()) {
            best_index = 0;

            for (size_t j = 0; j < base_plans_size; j++) {
                if (base_plan_selected[j]) {
                    continue;
                }
                auto duplicated_plan = base_plans[j]->duplicate();
                duplicated_plan->set_input_vars(input_vars);
                auto cost = duplicated_plan->estimate_cost();

                if (cost < best_cost) {
                    best_cost = cost;
                    best_index = j;
                    best_step_plan = move(duplicated_plan);
                }
            }
        }
        base_plan_selected[best_index] = true;
        input_vars |= best_step_plan->get_vars();
        // cout << "selected index " << best_index << "\n";
        ordered_plans.push_back(move(best_step_plan));
    }

    // set intersection and enumeration vars
    for (const auto& plan : ordered_plans) {
        auto encoded_vars = plan->get_vars();
        for (std::size_t i = 0; i < binding_size; i++) {
            if ((encoded_vars & (1UL << i)) != 0) {
                VarId var(i);
                // only consider variables not present in assigned_vars
                if (assigned_vars_set.find(var) == assigned_vars_set.end()) {
                    // Var is in enumeration_vars
                    if (enumeration_vars.find(var) != enumeration_vars.end()) {
                        enumeration_vars.erase(var);
                        intersection_vars_set.insert(var);
                        intersection_vars.push_back(var);
                    }
                    // Var is not in intersection vars
                    else if (intersection_vars_set.find(var) == intersection_vars_set.end()) {
                        enumeration_vars.insert(var);
                    }
                    // else var is already in intersection_vars, do nothing
                }
            }
        }
    }

    vector<VarId> var_order = intersection_vars; // TODO: move?
    for (const auto& var : enumeration_vars) {
        var_order.push_back(var);
    }

    const auto enumeration_level = intersection_vars.size();

    cout << "Var order: [";
    for (const auto& var : var_order) {
        cout << " " << var_names[var.id] << "(" << var.id << ")";
    }
    cout << " ]\n";

    cout << "   enumeration_level: " << enumeration_level << "\n";

    // Segunda pasada ahora si creando los LFIters
    vector<unique_ptr<LeapfrogIter>> leapfrog_iters;
    for (const auto& plan : base_plans) {
        auto lf_iter = plan->get_leapfrog_iter(thread_info, assigned_vars, var_order, enumeration_level);
        if (lf_iter == nullptr) {
            return nullptr;
        } else {
            leapfrog_iters.push_back(move(lf_iter));
        }
    }

    // At this point we know it won't return nullptr so we can change assigned_vars
    for (const auto& var : var_order) {
        assigned_vars.insert(var);
    }

    return make_unique<LeapfrogJoin>(move(leapfrog_iters), move(var_order), enumeration_level);
}


// TODO: a visitor would be nice
ObjectId BindingIdIterVisitor::get_value_id(const common::ast::Value& value) {
    if (value.type() == typeid(string)) {
        auto str = boost::get<string>(value);
        return model.get_object_id(GraphObject::make_string(str.c_str()));
    }
    else if (value.type() == typeid(int64_t)) {
        return model.get_object_id(GraphObject::make_int( boost::get<int64_t>(value) ));
    }
    else if (value.type() == typeid(float)) {
        return model.get_object_id(GraphObject::make_float( boost::get<float>(value) ));
    }
    else if (value.type() == typeid(bool)) {
        return model.get_object_id(GraphObject::make_bool( boost::get<bool>(value) ));
    }
    else {
        throw logic_error("Unknown value type.");
    }
}
