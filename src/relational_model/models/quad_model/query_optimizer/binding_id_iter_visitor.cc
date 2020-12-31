#include "binding_id_iter_visitor.h"

#include <cassert>
#include <iostream> // TODO: delete

#include "base/parser/logical_plan/op/op_match.h"
#include "base/parser/logical_plan/op/op_optional.h"
#include "relational_model/execution/binding_id_iter/optional_node.h"
#include "relational_model/models/quad_model/query_optimizer/join_plan/label_plan.h"
#include "relational_model/models/quad_model/query_optimizer/join_plan/nested_loop_plan.h"
#include "relational_model/models/quad_model/query_optimizer/join_plan/connection_plan.h"
#include "relational_model/models/quad_model/query_optimizer/join_plan/property_plan.h"
#include "relational_model/models/quad_model/query_optimizer/join_plan/unjoint_object_plan.h"
#include "relational_model/models/quad_model/query_optimizer/selinger_optimizer.h"

using namespace std;

BindingIdIterVisitor::BindingIdIterVisitor(QuadModel& model, const map<string, VarId>& var_name2var_id) :
    model           (model),
    var_name2var_id (var_name2var_id) { }

VarId BindingIdIterVisitor::get_var_id(const std::string& var) {
    auto search = var_name2var_id.find(var);
    if (search != var_name2var_id.end()) {
        return (*search).second;
    } else {
        throw std::logic_error("variable " + var + " not present in var_name2var_id");
        // VarId res(var_name2var_id.size());
        // var_name2var_id.insert({ var, res });
        // return res;
    }
}


void BindingIdIterVisitor::visit(const OpMatch& op_match) {
    // assigned_vars

    
    // new_assigned_vars
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
    /*
    if (base_plans.size() <= MAX_SELINGER_PLANS) {
        // TODO: construir aca var_names
        auto selinger_optimizer = SelingerOptimizer(move(base_plans), move(var_names));
        binding_id_iter = selinger_optimizer.get_binding_id_iter(binding_size);
    } else {
        binding_id_iter = get_greedy_join_plan( move(base_plans), binding_size);
    }
    */

    assert(tmp == nullptr);
    // TODO: use assigned_vars


    printf("BINDING SIZE (greedy join plan): %d\n", var_name2var_id.size());
    tmp = get_greedy_join_plan( move(base_plans), var_name2var_id.size() );
}


void BindingIdIterVisitor::visit(const OpOptional& op_optional) {
    // TODO: puede venir assigned_vars no vacio
    op_optional.op->accept_visitor(*this);
    unique_ptr<BindingIdIter> binding_id_iter = move(tmp);
    // TODO: al visitar el main pattern se asignan nuevas variables
    // suponer que cada visitor deja asignado assigned_vars correctamente?

    vector<unique_ptr<BindingIdIter>> optional_children;
    // TODO: Verify this idea works testing with complex optionals patterns
    auto current_scope_assigned_vars = assigned_vars; // TODO: maybe its not necesary if we only support well designed patterns
    // TODO: DELETE
    cout << "starting assigned vars from op_optional:\n";
    for (auto assigned_var :assigned_vars)
    {
        // FIND KEY
        string key = "";
        for (auto &i : var_name2var_id) {
            if (i.second.id == assigned_var.id) {
                key = i.first;
                break; // to stop searching
            }
        }
        // END FIND KEY
        cout << "assigned var: " << assigned_var.id << " (" << key << ")" << "\n";
    }
    // TODO: END DELETE
    for (auto& optional : op_optional.optionals) {
        optional->accept_visitor(*this); // P1 OPT {P2} OPT {P3} -> OpOptional(op: P1, optionals = {P2, P3})
                                         // P1 OPT {P2 OPT {P3}} -> OpOptional(op: P1, optionals = {OpOptional(op: P2, optionals: {P3})})
        optional_children.push_back(move(tmp));
        assigned_vars = current_scope_assigned_vars;
    }

    // TODO: we have to know the final binding size?
    auto binding_size = var_name2var_id.size();
    assert(tmp == nullptr);
    printf("optional node size: %d\n", optional_children.size());
    tmp = make_unique<OptionalNode>(binding_size, move(binding_id_iter), move(optional_children));
}


unique_ptr<BindingIdIter> BindingIdIterVisitor::get_greedy_join_plan(
    vector<unique_ptr<JoinPlan>> base_plans,
    std::size_t binding_size)
{
    auto base_plans_size = base_plans.size();
    assert(base_plans_size > 0);

    vector<string> var_names;
    var_names.resize(var_name2var_id.size());
    for (auto&& [var_name, var_id] : var_name2var_id) {
        var_names[var_id.id] = var_name;
    }

    // construct input vars;
    uint64_t input_vars = 0;
    assert(assigned_vars.size() <= 64 && "for now, a maximum of 64 variables is supported");
    for (auto var_id : assigned_vars) {
        input_vars |= 1UL << var_id.id;
    }

    // choose the first scan
    int best_index = 0;
    double best_cost = std::numeric_limits<double>::max();
    for (size_t j = 0; j < base_plans_size; j++) {
        base_plans[j]->set_input_vars(input_vars);
        auto current_element_cost = base_plans[j]->estimate_cost();
        // cout << j << ", cost:" << current_element_cost << ". ";
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
    std::cout << "\nPlan Generated:\n";
    root_plan->print(2, true, var_names);
    std::cout << "\nestimated cost: " << root_plan->estimate_cost() << "\n";

    const auto new_assigned_vars = root_plan->get_vars();
    for (uint64_t i = 0; i < sizeof(uint64_t); i++) {
        if ((new_assigned_vars & (1UL << i)) != 0) {
            assigned_vars.insert(VarId(i));
        }
    }

    return root_plan->get_binding_id_iter(binding_size);
}


ObjectId BindingIdIterVisitor::get_value_id(const common::ast::Value& value) {
    if (value.type() == typeid(string)) {
        return model.get_object_id(GraphObject::make_string( boost::get<string>(value) ));
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


void BindingIdIterVisitor::visit(const OpLabel&) { }
void BindingIdIterVisitor::visit(const OpProperty&) { }
void BindingIdIterVisitor::visit(const OpConnection&) { }
void BindingIdIterVisitor::visit(const OpConnectionType&) { }
void BindingIdIterVisitor::visit(const OpTransitiveClosure&) { }
void BindingIdIterVisitor::visit(const OpUnjointObject&) { }
void BindingIdIterVisitor::visit(const OpGraphPatternRoot&) { }
void BindingIdIterVisitor::visit(const OpSelect&) { }
void BindingIdIterVisitor::visit(const OpFilter&) { }
void BindingIdIterVisitor::visit(const OpOrderBy&) { }
void BindingIdIterVisitor::visit(const OpGroupBy&) { }
