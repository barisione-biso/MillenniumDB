#include "binding_id_iter_visitor.h"

#include <cassert>
#include <iostream>

#include "base/parser/logical_plan/op/op_match.h"
#include "base/parser/logical_plan/op/op_optional.h"
#include "relational_model/execution/binding_id_iter/optional_node.h"
#include "relational_model/models/quad_model/query_optimizer/join_plan/connection_plan.h"
#include "relational_model/models/quad_model/query_optimizer/join_plan/label_plan.h"
#include "relational_model/models/quad_model/query_optimizer/join_plan/nested_loop_plan.h"
#include "relational_model/models/quad_model/query_optimizer/join_plan/property_plan.h"
#include "relational_model/models/quad_model/query_optimizer/join_plan/transitive_closure_plan.h"
#include "relational_model/models/quad_model/query_optimizer/join_plan/unjoint_object_plan.h"
#include "relational_model/models/quad_model/query_optimizer/selinger_optimizer.h"

using namespace std;

constexpr auto MAX_SELINGER_PLANS = 0;

BindingIdIterVisitor::BindingIdIterVisitor(QuadModel& model, const map<string, VarId>& var_name2var_id) :
    model           (model),
    var_name2var_id (var_name2var_id) { }


VarId BindingIdIterVisitor::get_var_id(const std::string& var) {
    auto search = var_name2var_id.find(var);
    if (search != var_name2var_id.end()) {
        return (*search).second;
    } else {
        throw std::logic_error("variable " + var + " not present in var_name2var_id");
    }
}


void BindingIdIterVisitor::visit(OpMatch& op_match) {
    vector<unique_ptr<JoinPlan>> base_plans;

    // Process Labels
    for (auto& op_label : op_match.labels) {
        auto label_id = model.get_string_id(op_label.label);

        if (op_label.node_name[0] == '?') {
            auto node_var_id = get_var_id(op_label.node_name);
            base_plans.push_back(
                make_unique<LabelPlan>(model, node_var_id, label_id)
            );
        } else {
            auto node_id = model.get_identifiable_object_id(op_label.node_name);
            base_plans.push_back(
                make_unique<LabelPlan>(model, node_id, label_id)
            );
        }
    }

    // Process properties from Match
    for (auto& op_property : op_match.properties) {
        auto key_id   = model.get_string_id(op_property.key);
        auto value_id = get_value_id(op_property.value);

        if (op_property.obj_name[0] == '?') {
            auto obj_var_id = get_var_id(op_property.obj_name);

            base_plans.push_back(
                make_unique<PropertyPlan>(model, obj_var_id, key_id, value_id)
            );
        } else {
            auto obj_id = model.get_identifiable_object_id(op_property.obj_name);
            base_plans.push_back(
                make_unique<PropertyPlan>(model, obj_id, key_id, value_id)
            );
        }
    }

    // Process UnjointObjects
    for (auto& unjoint_object : op_match.unjoint_objects) {
        auto obj_var_id = get_var_id(unjoint_object.obj_name);
        base_plans.push_back(
            make_unique<UnjointObjectPlan>(model, obj_var_id)
        );
    }

    // Process connections
    for (auto& op_connection : op_match.connections) {
        auto from_id = op_connection.from[0] == '?'
                        ? (JoinPlan::Id) get_var_id(op_connection.from)
                        : (JoinPlan::Id) model.get_identifiable_object_id(op_connection.from);

        auto to_id   = op_connection.to[0] == '?'
                        ? (JoinPlan::Id) get_var_id(op_connection.to)
                        : (JoinPlan::Id) model.get_identifiable_object_id(op_connection.to);

        auto edge_id = get_var_id(op_connection.edge);

        // connections must have exactly 1 type in this model
        auto connection_labels_found = 0;

        for (auto& op_connection_type : op_match.connection_types) {
            if (op_connection_type.edge == op_connection.edge) {
                ++connection_labels_found;
                if (connection_labels_found > 1) {
                    throw QuerySemanticException("Connections must have exactly 1 type when using QuadModel");
                } else if (op_connection_type.type[0] == '?') {
                    auto type_id = get_var_id(op_connection_type.type);
                    base_plans.push_back(
                        make_unique<ConnectionPlan>(model, from_id, to_id, type_id, edge_id)
                    );
                } else {
                    auto type_id = model.get_identifiable_object_id(op_connection_type.type);
                    base_plans.push_back(
                        make_unique<ConnectionPlan>(model, from_id, to_id, type_id, edge_id)
                    );
                }
            }
        }
        if (connection_labels_found == 0) {
            auto type_id = get_var_id(op_connection.edge + ":type");
            base_plans.push_back(
                make_unique<ConnectionPlan>(model, from_id, to_id, type_id, edge_id)
            );
        }
    }

    // TODO: property paths are only transitive closures for now
    for (auto& property_path : op_match.property_paths) {
        auto from_id = property_path.from[0] == '?'
                        ? (JoinPlan::Id) get_var_id(property_path.from)
                        : (JoinPlan::Id) model.get_identifiable_object_id(property_path.from);

        auto to_id   = property_path.to[0] == '?'
                        ? (JoinPlan::Id) get_var_id(property_path.to)
                        : (JoinPlan::Id) model.get_identifiable_object_id(property_path.to);

        auto type_id = model.get_identifiable_object_id(property_path.type);

        base_plans.push_back(
            make_unique<TransitiveClosurePlan>(model, from_id, to_id, type_id)
        );
    }

    assert(tmp == nullptr);

    // construct var names
    vector<string> var_names;
    var_names.resize(var_name2var_id.size());
    for (auto&& [var_name, var_id] : var_name2var_id) {
        var_names[var_id.id] = var_name;
    }

    // construct input vars
    uint64_t input_vars = 0;
    assert(assigned_vars.size() <= 64 && "for now, a maximum of 64 variables is supported");
    for (auto var_id : assigned_vars) {
        input_vars |= 1UL << var_id.id;
    }

    unique_ptr<JoinPlan> root_plan = nullptr;
    if (base_plans.size() == 0) {
        throw QuerySemanticException("Empty plan");
    }

    if (base_plans.size() <= MAX_SELINGER_PLANS) {
        SelingerOptimizer selinger_optimizer(move(base_plans), var_names, input_vars);
        root_plan = selinger_optimizer.get_plan();
    } else {
        root_plan = get_greedy_join_plan(move(base_plans), var_names, input_vars);
    }

    auto binding_size = var_name2var_id.size();

    // insert new assigned_vars
    const auto new_assigned_vars = root_plan->get_vars();
    for (std::size_t i = 0; i < binding_size; i++) {
        if ((new_assigned_vars & (1UL << i)) != 0) {
            assigned_vars.insert(VarId(i));
        }
    }

    std::cout << "\nPlan Generated:\n";
    root_plan->print(2, true, var_names);
    std::cout << "\nestimated cost: " << root_plan->estimate_cost() << "\n";

    tmp = root_plan->get_binding_id_iter(binding_size);
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

    auto binding_size = var_name2var_id.size();
    assert(tmp == nullptr);
    tmp = make_unique<OptionalNode>(binding_size, move(binding_id_iter), move(optional_children));
}


unique_ptr<JoinPlan> BindingIdIterVisitor::get_greedy_join_plan(
    vector<unique_ptr<JoinPlan>> base_plans,
    vector<string>& var_names,
    uint64_t input_vars)
{
    auto base_plans_size = base_plans.size();
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


void BindingIdIterVisitor::visit(OpLabel&) { }
void BindingIdIterVisitor::visit(OpProperty&) { }
void BindingIdIterVisitor::visit(OpConnection&) { }
void BindingIdIterVisitor::visit(OpConnectionType&) { }
void BindingIdIterVisitor::visit(OpTransitiveClosure&) { }
void BindingIdIterVisitor::visit(OpUnjointObject&) { }
void BindingIdIterVisitor::visit(OpGraphPatternRoot&) { }
void BindingIdIterVisitor::visit(OpSelect&) { }
void BindingIdIterVisitor::visit(OpFilter&) { }
void BindingIdIterVisitor::visit(OpOrderBy&) { }
void BindingIdIterVisitor::visit(OpGroupBy&) { }
void BindingIdIterVisitor::visit(OpDistinct&) { }
