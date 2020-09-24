#include "query_optimizer.h"

#include <cassert>
#include <iostream>
#include <set>

#include "base/binding/binding.h"
#include "base/graph/value/value_int.h"
#include "base/graph/value/value_bool.h"
#include "base/graph/value/value_float.h"
#include "base/graph/value/value_string.h"
#include "base/parser/logical_plan/op/op_filter.h"
#include "base/parser/logical_plan/op/op_match.h"
#include "base/parser/logical_plan/op/op_select.h"
#include "base/parser/logical_plan/op/op_unjoint_object.h"

#include "relational_model/execution/binding_iter/filter.h"
#include "relational_model/execution/binding_iter/match.h"
#include "relational_model/execution/binding_iter/projection.h"

#include "relational_model/models/quad_model/query_optimizer/join_plan/join_plan.h"
#include "relational_model/models/quad_model/query_optimizer/join_plan/label_plan.h"
#include "relational_model/models/quad_model/query_optimizer/join_plan/nested_loop_plan.h"
#include "relational_model/models/quad_model/query_optimizer/join_plan/connection_plan.h"
#include "relational_model/models/quad_model/query_optimizer/join_plan/property_plan.h"
#include "relational_model/models/quad_model/query_optimizer/join_plan/unjoint_object_plan.h"
#include "relational_model/models/quad_model/query_optimizer/selinger_optimizer.h"

using namespace std;

QueryOptimizer::QueryOptimizer(QuadModel& model) :
    model(model) { }


unique_ptr<BindingIter> QueryOptimizer::exec(OpSelect& op_select) {
    op_select.accept_visitor(*this);
    return move(tmp);
}


void QueryOptimizer::visit(OpSelect& op_select) {
    if (op_select.select_items.size() == 0) {
        op_select.op->accept_visitor(*this);
        tmp = make_unique<Projection>(move(tmp), op_select.limit);
    }
    else {
        set<std::string> projection_vars;
        select_items = move(op_select.select_items);
        for (const auto& select_item : select_items) {
            if (select_item.key) {
                projection_vars.insert(select_item.var + '.' + select_item.key.get());
            } else {
                projection_vars.insert(select_item.var);
            }
        }
        op_select.op->accept_visitor(*this);
        tmp = make_unique<Projection>(move(tmp), move(projection_vars), op_select.limit);
    }
}


void QueryOptimizer::visit(OpMatch& op_match) {
    vector<unique_ptr<JoinPlan>> base_plans;

    // Process Labels
    for (auto& op_label : op_match.labels) {
        // auto graph_id = search_graph_id(op_node_label.graph_name);
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
        auto key_id     = model.get_string_id(op_property.key);
        auto value_id   = get_value_id(op_property.value);

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

    // Push properties from SELECT into MATCH
    for (const auto& select_item : select_items) {
        if (select_item.key) {
            auto obj_var_id = get_var_id(select_item.var);
            auto value_var  = get_var_id(select_item.var + '.' + select_item.key.get());
            auto key_id     = model.get_string_id(select_item.key.get());

            base_plans.push_back(
                make_unique<PropertyPlan>(model, obj_var_id, key_id, value_var)
            );
        }
    }

    // Process UnjointObjects not present in select
    for (auto& unjoint_object : op_match.unjoint_objects) {
        bool unjoint_object_mentioned_in_select = false;
        for (const auto& select_item : select_items) {
            if (select_item.var == unjoint_object.obj_name) {
                unjoint_object_mentioned_in_select = true;
                break;
            }
        }
        if (!unjoint_object_mentioned_in_select) {
            auto obj_var_id = get_var_id(unjoint_object.obj_name);
            base_plans.push_back(
                make_unique<UnjointObjectPlan>(model, obj_var_id)
            );
        }
    }

    // Process connections
    for (auto& op_connection : op_match.connections) {
        auto from_id = op_connection.from[0] == '?'
                        ? (JoinPlan::Id) get_var_id(op_connection.from)
                        : (JoinPlan::Id) model.get_identifiable_object_id(op_connection.from);

        auto to_id   = op_connection.to[0] == '?'
                        ? (JoinPlan::Id) get_var_id(op_connection.to)
                        : (JoinPlan::Id) model.get_identifiable_object_id(op_connection.to);

        auto edge_id = op_connection.edge[0] == '?'
                        ? (JoinPlan::Id) get_var_id(op_connection.edge)
                        : (JoinPlan::Id) model.get_identifiable_object_id(op_connection.edge);

        // connections must have exactly 1 type in this model
        auto connection_labels_found = 0;
        for (auto& op_connection_type : op_match.connection_types) {
            if (op_connection_type.edge == op_connection.edge) {
                auto type_id = model.get_identifiable_object_id(op_connection_type.type);
                ++connection_labels_found;
                if (connection_labels_found > 1) {
                    throw logic_error("Connections must have exactly 1 type when using QuadModel");
                } else {
                    base_plans.push_back(
                        make_unique<ConnectionPlan>(model, from_id, to_id, type_id, edge_id)
                    );
                }
            }
        }
        if (connection_labels_found == 0) {
            auto type_id = get_var_id(op_connection.edge + "_type");
            base_plans.push_back(
                make_unique<ConnectionPlan>(model, from_id, to_id, type_id, edge_id)
            );
        }
    }

    vector<string> var_names;
    var_names.resize(id_map.size());
    for (auto&& [var_name, var_id] : id_map) {
        var_names[var_id.id] = var_name;
    }
    auto selinger_optimizer = SelingerOptimizer(move(base_plans), move(var_names));
    tmp = make_unique<Match>(model, selinger_optimizer.get_binding_id_iter(), move(id_map));
    // tmp = make_unique<Match>(get_greedy_join_plan(move(base_plans)), move(id_map));
}


void QueryOptimizer::visit(OpFilter& op_filter) {
    op_filter.op->accept_visitor(*this);
    if (op_filter.condition != nullptr) {
        tmp = make_unique<Filter>(move(tmp), move(op_filter.condition));
    }
    // else tmp stays the same
}


VarId QueryOptimizer::get_var_id(const std::string& var) {
    auto search = id_map.find(var);
    if (id_map.find(var) != id_map.end()) {
        return (*search).second;
    }
    else {
        VarId res = VarId(id_count++);
        id_map.insert({ var, res });
        return res;
    }
}


ObjectId QueryOptimizer::get_value_id(const common::ast::Value& value) {
    if (value.type() == typeid(string)) {
        auto val_str = boost::get<string>(value);
        return model.get_value_id(ValueString(val_str));
    }
    else if (value.type() == typeid(int64_t)) {
        auto val_int = boost::get<int64_t>(value);
        return model.get_value_id(ValueInt(val_int));
    }
    else if (value.type() == typeid(float)) {
        auto val_float = boost::get<float>(value);
        return model.get_value_id(ValueFloat(val_float));
    }
    else if (value.type() == typeid(bool)) {
        auto val_bool = boost::get<bool>(value);
        return model.get_value_id(ValueBool(val_bool));
    }
    else {
        throw logic_error("Unknown value type.");
    }
}


unique_ptr<BindingIdIter> QueryOptimizer::get_greedy_join_plan(vector<unique_ptr<JoinPlan>> base_plans) {
    auto base_plans_size = base_plans.size();

    assert(base_plans_size > 0 && "base_plans size in Match must be greater than 0");

    vector<string> var_names;
    var_names.resize(id_map.size());
    for (auto&& [var_name, var_id] : id_map) {
        var_names[var_id.id] = var_name;
    }

    // choose the first scan
    int best_index = 0;
    double best_cost = std::numeric_limits<double>::max();
    for (size_t j = 0; j < base_plans_size; j++) {
        auto current_element_cost = base_plans[j]->estimate_cost();
        // cout << j << ", cost:" << current_element_cost << ". ";
        base_plans[j]->print(0, true, var_names);
        cout << "\n";
        if (current_element_cost < best_cost) {
            best_cost = current_element_cost;
            best_index = j;
        }
    }
    auto root_plan = move(base_plans[best_index]);

    // choose the next scan and make a IndexNestedLoppJoin or MergeJoin
    for (size_t i = 1; i < base_plans_size; i++) {
        best_index = 0;
        best_cost = std::numeric_limits<double>::max();
        unique_ptr<JoinPlan> best_step_plan = nullptr;

        for (size_t j = 0; j < base_plans_size; j++) {
            if (base_plans[j] != nullptr
                && !base_plans[j]->cartesian_product_needed(*root_plan) )
            {
                // auto merge_plan       =  make_unique<MergePlan>(root_plan->duplicate(), base_plans[j]->duplicate());
                auto nested_loop_plan =  make_unique<NestedLoopPlan>(root_plan->duplicate(), base_plans[j]->duplicate());

                // auto merge_cost       = merge_plan->estimate_cost();
                auto nested_loop_cost = nested_loop_plan->estimate_cost();

                // if (nested_loop_cost <= merge_cost) {
                    if (nested_loop_cost < best_cost) {
                        best_cost = nested_loop_cost;
                        best_index = j;
                        best_step_plan = move(nested_loop_plan);
                    }
                // } else { // merge_cost < nested_loop_cost
                //     if (merge_cost < best_cost) {
                //         best_cost = merge_cost;
                //         best_index = j;
                //         best_step_plan = move(merge_plan);
                //     }
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
                // auto merge_plan       =  make_unique<MergePlan>(root_plan->duplicate(), base_plans[j]->duplicate());
                auto nested_loop_plan =  make_unique<NestedLoopPlan>(root_plan->duplicate(), base_plans[j]->duplicate());

                // auto merge_cost       = merge_plan->estimate_cost();
                auto nested_loop_cost = nested_loop_plan->estimate_cost();

                // if (nested_loop_cost <= merge_cost) {
                    if (nested_loop_cost < best_cost) {
                        best_cost = nested_loop_cost;
                        best_index = j;
                        best_step_plan = move(nested_loop_plan);
                    }
                // } else { // merge_cost < nested_loop_cost
                //     if (merge_cost < best_cost) {
                //         best_cost = merge_cost;
                //         best_index = j;
                //         best_step_plan = move(merge_plan);
                //     }
                // }
            }
        }
        base_plans[best_index] = nullptr;
        root_plan = move(best_step_plan);
    }
    cout << "\nPlan Generated:\n";
    root_plan->print(2, true, var_names);
    cout << "\nestimated cost: " << root_plan->estimate_cost() << "\n";
    return root_plan->get_binding_id_iter();
}


unique_ptr<BindingIter> QueryOptimizer::exec(manual_plan_ast::Root& root) {
    // TODO: remake
    // vector<unique_ptr<JoinPlan>> base_plans;
    unique_ptr<JoinPlan> root_plan = nullptr;
    GraphId graph_id; // default graph

    // boost::variant<NodeLabel, EdgeLabel, NodeProperty, EdgeProperty, Connection, LabeledConnection>
    for (auto& relation : root.relations) {
        unique_ptr<JoinPlan> current_plan = nullptr;
        if (relation.type() == typeid(manual_plan_ast::NodeLabel)) {
            auto node_label = boost::get<manual_plan_ast::NodeLabel>(relation);

            auto node_var_id = get_var_id(node_label.node);
            auto label_id = model.get_string_id(node_label.label);

            current_plan = make_unique<LabelPlan>(model, node_var_id, label_id);
        }

        else if (relation.type() == typeid(manual_plan_ast::EdgeLabel)) {
            auto edge_label = boost::get<manual_plan_ast::EdgeLabel>(relation);

            auto edge_var_id = get_var_id(edge_label.edge);
            auto label_id = model.get_string_id(edge_label.label);

            current_plan = make_unique<LabelPlan>(model, edge_var_id, label_id);
        }

        else if (relation.type() == typeid(manual_plan_ast::NodeProperty)) {
            auto node_prop = boost::get<manual_plan_ast::NodeProperty>(relation);

            auto node_var_id = get_var_id(node_prop.node);
            auto key_id      = model.get_string_id(node_prop.key);
            auto value_id    = get_value_id(node_prop.value);

            current_plan = make_unique<PropertyPlan>(model, node_var_id, key_id, value_id);
        }

        if (relation.type() == typeid(manual_plan_ast::EdgeProperty)) {
            auto edge_prop = boost::get<manual_plan_ast::EdgeProperty>(relation);

            auto edge_var_id = get_var_id(edge_prop.edge);
            auto key_id      = model.get_string_id(edge_prop.key);
            auto value_id    = get_value_id(edge_prop.value);

            current_plan = make_unique<PropertyPlan>(model, edge_var_id, key_id, value_id);
        }

        else if (relation.type() == typeid(manual_plan_ast::Connection)) {
            auto connection = boost::get<manual_plan_ast::Connection>(relation);

            auto node_from_var_id = get_var_id(connection.node_from);
            auto edge_var_id      = get_var_id(connection.edge);
            auto node_to_var_id   = get_var_id(connection.node_to);

            current_plan = make_unique<ConnectionPlan>(model, node_from_var_id, node_to_var_id, edge_var_id, edge_var_id);
        }

        // else if (relation.type() == typeid(manual_plan_ast::LabeledConnection)) {
        //     auto labeled_connection = boost::get<manual_plan_ast::LabeledConnection>(relation);

        //     auto label_id = relational_model.get_string_id(labeled_connection.label);
        //     auto node_from_var_id = get_var_id(labeled_connection.node_from);
        //     auto node_to_var_id   = get_var_id(labeled_connection.node_to);
        //     auto edge_var_id      = get_var_id(labeled_connection.edge);

        //     current_plan = make_unique<LabeledConnectionPlan>(model, label_id, node_from_var_id, node_to_var_id, edge_var_id);
        // }

        if (root_plan == nullptr) {
            root_plan = move(current_plan);
        } else {
            root_plan = make_unique<NestedLoopPlan>(move(root_plan), move(current_plan));
        }
    }
    auto match = make_unique<Match>(model, root_plan->get_binding_id_iter(), move(id_map));
    return make_unique<Projection>(move(match), 0);
}


void QueryOptimizer::visit (OpLabel&) { }
void QueryOptimizer::visit (OpProperty&) { }
void QueryOptimizer::visit (OpConnection&) { }
void QueryOptimizer::visit (OpConnectionType&) { }
void QueryOptimizer::visit (OpUnjointObject&) { }
