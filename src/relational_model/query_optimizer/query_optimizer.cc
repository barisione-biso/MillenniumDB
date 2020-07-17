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
#include "base/parser/logical_plan/op/op_lonely_node.h"

#include "relational_model/relational_model.h"
#include "relational_model/execution/binding_iter/filter.h"
#include "relational_model/execution/binding_iter/match.h"
#include "relational_model/execution/binding_iter/projection.h"

#include "relational_model/query_optimizer/selinger_optimizer.h"
#include "relational_model/query_optimizer/join_plan/labeled_connection_plan.h"
#include "relational_model/query_optimizer/join_plan/connection_plan.h"
#include "relational_model/query_optimizer/join_plan/edge_label_plan.h"
#include "relational_model/query_optimizer/join_plan/edge_property_plan.h"
#include "relational_model/query_optimizer/join_plan/node_label_plan.h"
#include "relational_model/query_optimizer/join_plan/node_property_plan.h"
#include "relational_model/query_optimizer/join_plan/lonely_node_plan.h"
#include "relational_model/query_optimizer/join_plan/join_plan.h"
#include "relational_model/query_optimizer/join_plan/nested_loop_plan.h"
#include "relational_model/query_optimizer/join_plan/node_loop_plan.h"
// #include "relational_model/query_optimizer/join_plan/merge_plan.h"

#include "relational_model/manual_plan/grammar/manual_plan_ast.h"

#include "storage/catalog/catalog.h"

using namespace std;

QueryOptimizer::QueryOptimizer() { }


unique_ptr<BindingIter> QueryOptimizer::exec(OpSelect& op_select) {
    op_select.accept_visitor(*this);
    return move(tmp);
}


void QueryOptimizer::visit(OpSelect& op_select) {
    if (op_select.select_all) {
        op_select.op->accept_visitor(*this);
        tmp = make_unique<Projection>(move(tmp), op_select.limit);
    }
    else {
        set<std::string> projection_vars;
        select_items = move(op_select.select_items);
        for (auto&& [key, value] : select_items) {
            projection_vars.insert(key + '.' + value);
        }
        op_select.op->accept_visitor(*this);
        tmp = make_unique<Projection>(move(tmp), move(projection_vars), op_select.limit);
    }
}


void QueryOptimizer::visit(OpMatch& op_match) {
    vector<unique_ptr<JoinPlan>> base_plans;

    for (auto&& [var_name, graph_name] : op_match.var_name2graph_name) {
        auto graph_id = catalog.get_graph(graph_name);
        graph_ids.insert({ graph_name, graph_id }); // may try to insert a repeated pair, this is OK, it won't be duplicated
        var2graph_id.insert({ var_name, graph_id });
    }
    // element_types = op_match.var_name2type;
    node_names = op_match.node_names;
    edge_names = op_match.edge_names;

    // Process Labels
    for (auto& op_node_label : op_match.node_labels) {
        auto graph_id = search_graph_id(op_node_label.graph_name);
        auto node_var_id = get_var_id(op_node_label.node_name);
        auto label_id = relational_model.get_string_id(op_node_label.label);

        base_plans.push_back(
            make_unique<NodeLabelPlan>(graph_id, node_var_id, label_id)
        );
    }

    // order labels by cost using catalog
    vector<OpEdgeLabel> edge_labels(op_match.edge_labels.begin(), op_match.edge_labels.end());
    std::sort(edge_labels.begin(), edge_labels.end(), [this](const auto& lhs, const auto& rhs) {
        auto lhs_cost = catalog.get_edge_count_for_label(search_graph_id(lhs.graph_name),
                                                         relational_model.get_string_id(lhs.label));

        auto rhs_cost = catalog.get_edge_count_for_label(search_graph_id(rhs.graph_name),
                                                         relational_model.get_string_id(rhs.label));
        return lhs_cost < rhs_cost;
    });

    for (auto& op_edge_label : edge_labels) {
        auto graph_id    = search_graph_id(op_edge_label.graph_name);
        auto edge_var_id = get_var_id(op_edge_label.edge_name);
        auto label_id    = relational_model.get_string_id(op_edge_label.label);

        bool labeled_edge_added = false;

        for (auto it = op_match.connections.begin(); it != op_match.connections.end(); ++it) {
            if (it->edge == op_edge_label.edge_name) {
                auto graph_id         = search_graph_id(it->graph_name);
                auto node_from_var_id = get_var_id(it->node_from);
                auto node_to_var_id   = get_var_id(it->node_to);

                base_plans.push_back(
                    make_unique<LabeledConnectionPlan>(graph_id, label_id, node_from_var_id, node_to_var_id, edge_var_id)
                );
                labeled_edge_added = true;
                op_match.connections.erase(it);
                break;
            }
        }

        if (!labeled_edge_added) {
            base_plans.push_back(
                make_unique<EdgeLabelPlan>(graph_id, edge_var_id, label_id)
            );
        }
    }

    // Process properties from Match
    for (auto& op_node_property : op_match.node_properties) {
        auto node_var_id = get_var_id(op_node_property.node_name);
        auto key_id      = relational_model.get_string_id(op_node_property.key);
        auto value_id    = get_value_id(op_node_property.value);

        auto graph_id = search_graph_id(op_node_property.graph_name);
        base_plans.push_back(
            make_unique<NodePropertyPlan>(graph_id, node_var_id, key_id, value_id)
        );
    }

    for (auto& op_edge_property : op_match.edge_properties) {
        auto graph_id    = search_graph_id(op_edge_property.graph_name);
        auto edge_var_id = get_var_id(op_edge_property.edge_name);
        auto key_id      = relational_model.get_string_id(op_edge_property.key);
        auto value_id    = get_value_id(op_edge_property.value);

        base_plans.push_back(
            make_unique<EdgePropertyPlan>(graph_id, edge_var_id, key_id, value_id)
        );
    }

    // Process properties from select
    for (auto&& [var, key] : select_items) {
        auto graph_id       = var2graph_id[var];
        auto element_var_id = get_var_id(var);
        auto value_var      = get_var_id(var + '.' + key);
        auto key_id         = relational_model.get_string_id(key);

        // check if var is a node or an edge
        auto node_search = node_names.find(var);
        if (node_search != node_names.end()) {
            base_plans.push_back(
                make_unique<NodePropertyPlan>(graph_id, element_var_id, key_id, value_var)
            );
        } else {
            base_plans.push_back(
                make_unique<EdgePropertyPlan>(graph_id, element_var_id, key_id, value_var)
            );
        }
    }

    // Process Lonely Nodes not present in select
    for (auto& lonely_node : op_match.lonely_nodes) {
        bool lonely_node_mentioned_int_select = false;
        for (auto& pair : select_items) {
            if (pair.first == lonely_node.node_name) {
                lonely_node_mentioned_int_select = true;
                break;
            }
        }
        if (!lonely_node_mentioned_int_select) {
            auto graph_id = search_graph_id(lonely_node.graph_name);
            auto node_var_id = get_var_id(lonely_node.node_name);
            base_plans.push_back(
                make_unique<LonelyNodePlan>(graph_id, node_var_id)
            );
        }
    }

    // Process connections
    for (auto& op_connection : op_match.connections) {
        auto graph_id = search_graph_id(op_connection.graph_name);

        auto node_from_var_id = get_var_id(op_connection.node_from);
        auto node_to_var_id   = get_var_id(op_connection.node_to);
        auto edge_var_id      = get_var_id(op_connection.edge);

        base_plans.push_back(
            make_unique<ConnectionPlan>(graph_id, node_from_var_id, node_to_var_id, edge_var_id)
        );
    }
    for (auto& op_node_loop : op_match.node_loops) {
        auto graph_id = search_graph_id(op_node_loop.graph_name);

        auto node_id     = get_var_id(op_node_loop.node);
        auto edge_var_id = get_var_id(op_node_loop.edge);

        base_plans.push_back(
            make_unique<NodeLoopPlan>(graph_id, node_id, edge_var_id)
        );
    }

    vector<string> var_names;
    var_names.resize(id_map.size());
    for (auto&& [var_name, var_id] : id_map) {
        var_names[var_id.id] = var_name;
    }
    auto selinger_optimizer = SelingerOptimizer(move(base_plans), move(var_names));
    tmp = make_unique<Match>(selinger_optimizer.get_binding_id_iter(), move(id_map));
    // tmp = make_unique<Match>(get_greedy_join_plan(move(base_plans)), move(id_map));
}


void QueryOptimizer::visit(OpFilter& op_filter) {
    op_filter.op->accept_visitor(*this);
    if (op_filter.condition != nullptr) {
        tmp = make_unique<Filter>(move(tmp), move(op_filter.condition), var2graph_id, node_names, edge_names);
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


ObjectId QueryOptimizer::get_value_id(const ast::Value& value) {
    if (value.type() == typeid(string)) {
        auto val_str = boost::get<string>(value);
        return relational_model.get_value_id(ValueString(val_str));
    }
    else if (value.type() == typeid(int64_t)) {
        auto val_int = boost::get<int64_t>(value);
        return relational_model.get_value_id(ValueInt(val_int));
    }
    else if (value.type() == typeid(float)) {
        auto val_float = boost::get<float>(value);
        return relational_model.get_value_id(ValueFloat(val_float));
    }
    else if (value.type() == typeid(bool)) {
        auto val_bool = boost::get<bool>(value);
        return relational_model.get_value_id(ValueBool(val_bool));
    }
    else {
        throw logic_error("Unknown value type.");
    }
}


GraphId QueryOptimizer::search_graph_id(const std::string& graph_name) {
    auto search = graph_ids.find(graph_name);
    if (search != graph_ids.end()) {
        return search->second;
    } else {
        throw logic_error("graph_ids does not contain graph \"" + graph_name
            + "\", graph_ids should contain all graph names, non-existing graphs should be checked before.");
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
        cout << j << ", cost:" << current_element_cost << ". ";
        base_plans[j]->print(0, var_names);
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
    root_plan->print(2, var_names);
    cout << "\nestimated cost: " << root_plan->estimate_cost() << "\n";
    return root_plan->get_binding_id_iter();
}


unique_ptr<BindingIter> QueryOptimizer::exec(manual_plan_ast::Root& root) {
    // vector<unique_ptr<JoinPlan>> base_plans;
    unique_ptr<JoinPlan> root_plan = nullptr;
    GraphId graph_id; // default graph

    // boost::variant<NodeLabel, EdgeLabel, NodeProperty, EdgeProperty, Connection, LabeledConnection>
    for (auto& relation : root.relations) {
        unique_ptr<JoinPlan> current_plan = nullptr;
        if (relation.type() == typeid(manual_plan_ast::NodeLabel)) {
            auto node_label = boost::get<manual_plan_ast::NodeLabel>(relation);

            auto node_var_id = get_var_id(node_label.node_name);
            auto label_id = relational_model.get_string_id(node_label.label);

            current_plan = make_unique<NodeLabelPlan>(graph_id, node_var_id, label_id);
        }

        else if (relation.type() == typeid(manual_plan_ast::EdgeLabel)) {
            auto edge_label = boost::get<manual_plan_ast::EdgeLabel>(relation);

            auto edge_var_id = get_var_id(edge_label.edge_name);
            auto label_id = relational_model.get_string_id(edge_label.label);

            current_plan = make_unique<NodeLabelPlan>(graph_id, edge_var_id, label_id);
        }

        else if (relation.type() == typeid(manual_plan_ast::NodeProperty)) {
            auto node_prop = boost::get<manual_plan_ast::NodeProperty>(relation);

            auto node_var_id = get_var_id(node_prop.node_name);
            auto key_id      = relational_model.get_string_id(node_prop.key);
            auto value_id    = get_value_id(node_prop.value);

            current_plan = make_unique<NodePropertyPlan>(graph_id, node_var_id, key_id, value_id);
        }

        if (relation.type() == typeid(manual_plan_ast::EdgeProperty)) {
            auto edge_prop = boost::get<manual_plan_ast::EdgeProperty>(relation);

            auto edge_var_id = get_var_id(edge_prop.edge_name);
            auto key_id      = relational_model.get_string_id(edge_prop.key);
            auto value_id    = get_value_id(edge_prop.value);

            current_plan = make_unique<EdgePropertyPlan>(graph_id, edge_var_id, key_id, value_id);
        }

        else if (relation.type() == typeid(manual_plan_ast::Connection)) {
            auto connection = boost::get<manual_plan_ast::Connection>(relation);

            auto node_from_var_id = get_var_id(connection.node_from_name);
            auto edge_var_id      = get_var_id(connection.edge_name);
            auto node_to_var_id   = get_var_id(connection.node_to_name);

            current_plan = make_unique<ConnectionPlan>(graph_id, node_from_var_id, node_to_var_id, edge_var_id);
        }

        else if (relation.type() == typeid(manual_plan_ast::LabeledConnection)) {
            auto labeled_connection = boost::get<manual_plan_ast::LabeledConnection>(relation);

            auto label_id = relational_model.get_string_id(labeled_connection.label);
            auto node_from_var_id = get_var_id(labeled_connection.node_from_name);
            auto node_to_var_id   = get_var_id(labeled_connection.node_to_name);
            auto edge_var_id      = get_var_id(labeled_connection.edge_name);

            current_plan = make_unique<LabeledConnectionPlan>(graph_id, label_id, node_from_var_id, node_to_var_id, edge_var_id);
        }

        if (root_plan == nullptr) {
            root_plan = move(current_plan);
        } else {
            root_plan = make_unique<NestedLoopPlan>(move(root_plan), move(current_plan));
        }
    }
    auto match = make_unique<Match>(root_plan->get_binding_id_iter(), move(id_map));
    return make_unique<Projection>(move(match), 0);
}


void QueryOptimizer::visit (OpNodeLabel&) { }
void QueryOptimizer::visit (OpEdgeLabel&) { }
void QueryOptimizer::visit (OpNodeProperty&) { }
void QueryOptimizer::visit (OpEdgeProperty&) { }
void QueryOptimizer::visit (OpConnection&) { }
void QueryOptimizer::visit (OpLonelyNode&) { }
void QueryOptimizer::visit (OpNodeLoop&) { }
