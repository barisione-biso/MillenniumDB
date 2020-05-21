#include "physical_plan_generator.h"

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
#include "relational_model/physical_plan/binding_iter/filter.h"
#include "relational_model/physical_plan/binding_iter/match.h"
#include "relational_model/physical_plan/binding_iter/projection.h"

#include "relational_model/query_optimizer/join_plan/connection_plan.h"
#include "relational_model/query_optimizer/join_plan/edge_label_plan.h"
#include "relational_model/query_optimizer/join_plan/edge_property_plan.h"
#include "relational_model/query_optimizer/join_plan/node_label_plan.h"
#include "relational_model/query_optimizer/join_plan/node_property_plan.h"
#include "relational_model/query_optimizer/join_plan/lonely_node_plan.h"

#include "storage/catalog/catalog.h"

using namespace std;

PhysicalPlanGenerator::PhysicalPlanGenerator() { }


unique_ptr<BindingIter> PhysicalPlanGenerator::exec(OpSelect& op_select) {
    op_select.accept_visitor(*this);
    return move(tmp);
}


void PhysicalPlanGenerator::visit(OpSelect& op_select) {
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


void PhysicalPlanGenerator::visit(OpMatch& op_match) {
    VarId null_var = VarId::get_null();
    vector<unique_ptr<JoinPlan>> base_plans;

    for (auto&& [var_name, graph_name] : op_match.var_name2graph_name) {
        auto graph_id = catalog.get_graph(graph_name);
        graph_ids.insert({ graph_name, graph_id }); // may try to insert a repeated pair, this is OK, it won't be duplicated
        var2graph_id.insert({ var_name, graph_id });
    }
    element_types =  op_match.var_name2type;

    // Process Labels
    for (auto& op_label : op_match.labels) {
        auto graph_id = search_graph_id(op_label->graph_name);
        auto element_var_id = get_var_id(op_label->var);
        auto label_id = relational_model.get_string_unmasked_id(op_label->label);

        if (op_label->type == ObjectType::node) {
            base_plans.push_back(
                make_unique<NodeLabelPlan>(graph_id, element_var_id, null_var, label_id)
            );
        } else {
            base_plans.push_back(
                make_unique<EdgeLabelPlan>(graph_id, element_var_id, null_var, label_id)
            );
        }
    }

    // Process properties from Match
    for (auto& op_property : op_match.properties) {
        auto element_var_id = get_var_id(op_property->var);
        auto key_id = relational_model.get_string_unmasked_id(op_property->key);
        ObjectId value_id = get_value_id(op_property->value);

        auto graph_id = search_graph_id(op_property->graph_name);
        if (op_property->type == ObjectType::node) {
            base_plans.push_back(
                make_unique<NodePropertyPlan>(graph_id, element_var_id, null_var, null_var, key_id, value_id)
            );
        } else {
            base_plans.push_back(
                make_unique<EdgePropertyPlan>(graph_id, element_var_id, null_var, null_var, key_id, value_id)
            );
        }
    }

    // Process properties from select
    for (auto&& [var, key] : select_items) {
        auto graph_id = var2graph_id[var];
        auto element_var_id = get_var_id(var);
        auto value_var = get_var_id(var + '.' + key);
        auto key_id = relational_model.get_string_unmasked_id(key);
        auto element_type = element_types[var];

        if (element_type == ObjectType::node) {
            base_plans.push_back(
                make_unique<NodePropertyPlan>(graph_id, element_var_id, null_var, value_var, key_id,
                                              ObjectId::get_null())
            );
        } else {
            base_plans.push_back(
                make_unique<EdgePropertyPlan>(graph_id, element_var_id, null_var, value_var, key_id,
                                              ObjectId::get_null())
            );
        }
    }

    // Process Lonely Nodes not present in select
    for (auto& lonely_node : op_match.lonely_nodes) {
        bool lonely_node_mentioned_int_select = false;
        for (auto& pair : select_items) {
            if (pair.first == lonely_node->var) {
                lonely_node_mentioned_int_select = true;
                break;
            }
        }
        if (!lonely_node_mentioned_int_select) {
            auto graph_id = search_graph_id(lonely_node->graph_name);
            auto element_var_id = get_var_id(lonely_node->var);
            base_plans.push_back(
                make_unique<LonelyNodePlan>(graph_id, element_var_id)
            );
        }
    }

    // Process connections
    for (auto& op_connection : op_match.connections) {
        auto graph_id = search_graph_id(op_connection->graph_name);

        auto node_from_var_id = get_var_id(op_connection->node_from);
        auto node_to_var_id   = get_var_id(op_connection->node_to);
        auto edge_var_id      = get_var_id(op_connection->edge);

        base_plans.push_back(
            make_unique<ConnectionPlan>(graph_id, node_from_var_id, node_to_var_id, edge_var_id)
        );
    }

    // TODO: decidir join order y pasarselo al match
    tmp = make_unique<Match>(move(base_plans), move(id_map));
}


void PhysicalPlanGenerator::visit(OpFilter& op_filter) {
    op_filter.op->accept_visitor(*this);
    if (op_filter.condition != nullptr) {
        tmp = make_unique<Filter>(move(tmp), move(op_filter.condition), var2graph_id, element_types);
    }
    // else tmp stays the same
}


VarId PhysicalPlanGenerator::get_var_id(const std::string& var) {
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


ObjectId PhysicalPlanGenerator::get_value_id(const ast::Value& value) {
    if (value.type() == typeid(string)) {
        auto val_str = boost::get<string>(value);
        return relational_model.get_value_masked_id(ValueString(val_str));
    }
    else if (value.type() == typeid(int)) {
        auto val_int = boost::get<int>(value);
        return relational_model.get_value_masked_id(ValueInt(val_int));
    }
    else if (value.type() == typeid(float)) {
        auto val_float = boost::get<float>(value);
        return relational_model.get_value_masked_id(ValueFloat(val_float));
    }
    else if (value.type() == typeid(bool)) {
        auto val_bool = boost::get<bool>(value);
        return relational_model.get_value_masked_id(ValueBool(val_bool));
    }
    else {
        throw logic_error("Unknown value type.");
    }
}


GraphId PhysicalPlanGenerator::search_graph_id(const std::string& graph_name) {
    auto search = graph_ids.find(graph_name);
    if (search != graph_ids.end()) {
        return search->second;
    } else {
        throw logic_error("graph_ids does not contain graph \"" + graph_name
            + "\", graph_ids should contain all graph names, non-existing graphs should be checked before.");
    }
}


void PhysicalPlanGenerator::visit (OpLabel&) { }
void PhysicalPlanGenerator::visit (OpProperty&) { }
void PhysicalPlanGenerator::visit (OpConnection&) { }
void PhysicalPlanGenerator::visit (OpLonelyNode&) { }
