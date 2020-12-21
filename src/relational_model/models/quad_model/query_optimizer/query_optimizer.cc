#include "query_optimizer.h"

#include <cassert>
#include <iostream>
#include <set>

#include "base/binding/binding.h"
#include "base/parser/logical_plan/op/op_filter.h"
#include "base/parser/logical_plan/op/op_match.h"
#include "base/parser/logical_plan/op/op_select.h"
#include "base/parser/logical_plan/op/op_unjoint_object.h"
#include "base/parser/logical_plan/op/op_order_by.h"
#include "base/parser/logical_plan/op/op_group_by.h"
#include "base/parser/logical_plan/op/visitors/formula_to_condition.h"

#include "relational_model/execution/binding_iter/match.h"
#include "relational_model/execution/binding_iter/select.h"
#include "relational_model/execution/binding_iter/where.h"

// CRIS INCLUDES
#include "relational_model/execution/binding_id_iter/left_outer_join.h"
#include "relational_model/execution/binding_id_iter/optional_node.h"
#include "relational_model/execution/binding_id_iter/index_nested_loop_join.h" // delete
#include "relational_model/execution/binding_id_iter/index_scan.h"
#include "base/ids/object_id.h"
#include "base/ids/var_id.h"
// END CRIS INCLUDES

#include "relational_model/models/quad_model/query_optimizer/join_plan/join_plan.h"
#include "relational_model/models/quad_model/query_optimizer/join_plan/label_plan.h"
#include "relational_model/models/quad_model/query_optimizer/join_plan/nested_loop_plan.h"
#include "relational_model/models/quad_model/query_optimizer/join_plan/connection_plan.h"
#include "relational_model/models/quad_model/query_optimizer/join_plan/property_plan.h"
#include "relational_model/models/quad_model/query_optimizer/join_plan/unjoint_object_plan.h"
#include "relational_model/models/quad_model/query_optimizer/selinger_optimizer.h"

constexpr auto MAX_SELINGER_PLANS = 1;

using namespace std;

QueryOptimizer::QueryOptimizer(QuadModel& model) :
    model(model) { }


unique_ptr<BindingIter> QueryOptimizer::exec(OpSelect& op_select) {
    op_select.accept_visitor(*this);
    return move(tmp);
}


void QueryOptimizer::visit(const OpSelect& op_select) {
    // need to remember to be able to push properties from select to match
    select_items = move(op_select.select_items);
    op_select.op->accept_visitor(*this);

    vector<pair<string, VarId>> projection_vars;
    for (const auto& select_item : select_items) {
        string var_name = select_item.var;
        if (select_item.key) {
            var_name += '.';
            var_name += select_item.key.get();
        }
        auto var_id = get_var_id(var_name);
        projection_vars.push_back(make_pair(var_name, var_id));
    }

    if (projection_vars.size() == 0) {
        // SELECT *
        for (auto&& [k, v] : id_map) {
            projection_vars.push_back(make_pair(k, v));
        }
    }

    tmp = make_unique<Select>(move(tmp), move(projection_vars), op_select.limit);
}


void QueryOptimizer::visit(const OpMatch& op_match) {
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

    // Process UnjointObjects when select doesn't project a property of them.
    // We need to discard UnjointObjects like ?x in:
    // SELECT ?x.name
    // MATCH (?x)
    for (auto& unjoint_object : op_match.unjoint_objects) {
        bool unjoint_object_mentioned_in_select = false;
        for (const auto& select_item : select_items) {
            if (select_item.key && select_item.var == unjoint_object.obj_name) {
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
            auto type_id = get_var_id(op_connection.edge + ".type");
            base_plans.push_back(
                make_unique<ConnectionPlan>(model, from_id, to_id, type_id, edge_id)
            );
        }
    }

    // TODO: Process property paths
    for (auto& property_path : op_match.property_paths) {
        throw QuerySemanticException("Property paths not implemented yet");
    }


    vector<string> var_names;
    var_names.resize(id_map.size());
    for (auto&& [var_name, var_id] : id_map) {
        var_names[var_id.id] = var_name;
    }

    // get binding size
    unique_ptr<BindingIdIter> binding_id_iter;

    for (auto& property_path : op_match.property_paths) {
        throw QuerySemanticException("Property paths not implemented yet");
    }

    var_names.resize(id_map.size());
    for (auto&& [var_name, var_id] : id_map) {
        var_names[var_id.id] = var_name;
    }

    for (const auto& select_item : select_items) {
        if (select_item.key) {
            // This is for filling up id_map in order to know the binding_size
            get_var_id(select_item.var);
            get_var_id(select_item.var + '.' + select_item.key.get());
        }
    }

    // final size
    auto binding_size = id_map.size();

    // get initial binding_id_iter
    if (base_plans.size() <= MAX_SELINGER_PLANS) {
        auto selinger_optimizer = SelingerOptimizer(move(base_plans), move(var_names));
        binding_id_iter = selinger_optimizer.get_binding_id_iter(binding_size);
    } else {
        binding_id_iter = get_greedy_join_plan( move(base_plans), binding_size);
    }

    // vector of binding_id_iter
    vector<unique_ptr<BindingIdIter>> opt_children;

    // Push properties from SELECT into MATCH
    for (const auto& select_item : select_items) {
        if (select_item.key) {
            auto obj_var_id = get_var_id(select_item.var);
            auto value_var  = get_var_id(select_item.var + '.' + select_item.key.get());

            // TODO: check property is not present in Match
            // TODO: explicar porque funciona mal si no se vuelve a meter el property en el match
            auto key_id = model.get_string_id(select_item.key.get());

            array<unique_ptr<ScanRange>, 3> ranges;

            ranges[0] = get_scan_range(obj_var_id, true);
            ranges[1] = get_scan_range(key_id, true);
            ranges[2] = get_scan_range(value_var, false);
            auto index_scan = make_unique<IndexScan<3>>(binding_size, *model.object_key_value, move(ranges));
            // binding_id_iter = make_unique<LeftOuterJoin>(binding_size, move(binding_id_iter), move(index_scan));
            opt_children.push_back(move(index_scan));

        }
    }
    // Optional node
    binding_id_iter =  make_unique<OptionalNode>(binding_size, move(binding_id_iter), move(opt_children)); // NEW
    tmp = make_unique<Match>(model, move(binding_id_iter), binding_size);
}


void QueryOptimizer::visit(const OpFilter& op_filter) {
    op_filter.op->accept_visitor(*this);
    auto match_binding_size = id_map.size();

    Formula2ConditionVisitor visitor(model, id_map);
    auto condition = visitor(op_filter.formula);
    auto new_property_var_id = move(visitor.property_map);

    tmp = make_unique<Where>(
        model,
        move(tmp),
        move(condition),
        match_binding_size,
        move(new_property_var_id)
    );
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


std::unique_ptr<ScanRange> QueryOptimizer::get_scan_range(Id id, bool assigned) {
    if ( std::holds_alternative<ObjectId>(id) ) {
        return std::make_unique<Term>(std::get<ObjectId>(id));
    } else if (assigned) {
        return std::make_unique<AssignedVar>(std::get<VarId>(id));
    } else {
        return std::make_unique<UnassignedVar>(std::get<VarId>(id));
    }
}


ObjectId QueryOptimizer::get_value_id(const common::ast::Value& value) {
    if (value.type() == typeid(string)) {
        auto str = boost::get<string>(value);
        return model.get_object_id(GraphObject::make_string(str));
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


unique_ptr<BindingIdIter> QueryOptimizer::get_greedy_join_plan(
    vector<unique_ptr<JoinPlan>> base_plans,
    std::size_t binding_size)
{
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
        std::cout << "\n";
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
                auto nested_loop_plan =  make_unique<NestedLoopPlan>(root_plan->duplicate(), base_plans[j]->duplicate());

                // auto merge_cost       = merge_plan->estimate_cost();
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
                auto nested_loop_plan =  make_unique<NestedLoopPlan>(root_plan->duplicate(), base_plans[j]->duplicate());

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
    return root_plan->get_binding_id_iter(binding_size);
}


unique_ptr<BindingIter> QueryOptimizer::exec(manual_plan::ast::ManualRoot& root) {
    unique_ptr<JoinPlan> root_plan = nullptr;

    for (auto& relation : root.relations) {
        unique_ptr<JoinPlan> current_plan = nullptr;
        if (relation.type() == typeid(manual_plan::ast::NodeLabel)) {
            auto node_label = boost::get<manual_plan::ast::NodeLabel>(relation);

            auto var_id   = get_var_id(node_label.var_or_id);
            auto label_id = model.get_string_id(node_label.label);

            if (node_label.var_or_id[0] == '?') {
                auto node_var_id = get_var_id(node_label.var_or_id);
                current_plan = make_unique<LabelPlan>(model, node_var_id, label_id);
            } else {
                auto node_id = model.get_identifiable_object_id(node_label.var_or_id);
                current_plan = make_unique<LabelPlan>(model, node_id, label_id);
            }
        }

        else if (relation.type() == typeid(manual_plan::ast::ObjectProperty)) {
            auto property = boost::get<manual_plan::ast::ObjectProperty>(relation);

            auto key_id   = model.get_string_id(property.key);
            auto value_id = get_value_id(property.value);

            if (property.var_or_id[0] == '?') {
                auto obj_var_id = get_var_id(property.var_or_id);
                current_plan = make_unique<PropertyPlan>(model, obj_var_id, key_id, value_id);
            } else {
                auto obj_id = model.get_identifiable_object_id(property.var_or_id);
                current_plan = make_unique<PropertyPlan>(model, obj_id, key_id, value_id);
            }
        }

        else if (relation.type() == typeid(manual_plan::ast::TypedConnection)) {
            auto connection = boost::get<manual_plan::ast::TypedConnection>(relation);

            auto from_id = connection.from[0] == '?'
                         ? (JoinPlan::Id) get_var_id(connection.from)
                         : (JoinPlan::Id) model.get_identifiable_object_id(connection.from);

            auto to_id   = connection.to[0] == '?'
                         ? (JoinPlan::Id) get_var_id(connection.to)
                         : (JoinPlan::Id) model.get_identifiable_object_id(connection.to);

            auto type_id = connection.type[0] == '?'
                         ? (JoinPlan::Id) get_var_id(connection.type)
                         : (JoinPlan::Id) model.get_identifiable_object_id(connection.type);

            auto edge_id = get_var_id(connection.edge);

            current_plan = make_unique<ConnectionPlan>(model, from_id, to_id, type_id, edge_id);
        }
        else {
            throw QuerySemanticException("ManualPlan typeid not implemented in Query Optimizer yet");
        }

        if (root_plan == nullptr) {
            root_plan = move(current_plan);
        } else {
            root_plan = make_unique<NestedLoopPlan>(move(root_plan), move(current_plan));
        }
    }
    auto binding_size = id_map.size();
    auto match = make_unique<Match>(model, root_plan->get_binding_id_iter(binding_size), binding_size);
    vector<pair<string, VarId>> projection_vars; // empty list will select *
    for (const auto& select_item : root.selection) {
        string var_name = select_item.var;
        if (select_item.key) {
            var_name += '.';
            var_name += select_item.key.get();
        }
        auto var_id = get_var_id(var_name);
        projection_vars.push_back(make_pair(var_name, var_id));
    }

    if (projection_vars.size() == 0) {
        // SELECT *
        for (auto&& [k, v] : id_map) {
            projection_vars.push_back(make_pair(k, v));
        }
    }
    uint_fast32_t limit = root.limit ? root.limit.get() : 0;
    return make_unique<Select>(move(match), move(projection_vars), limit);
}


void QueryOptimizer::visit(const OpGroupBy& op_group_by) {
    op_group_by.op->accept_visitor(*this);
}


void QueryOptimizer::visit(const OpOrderBy& order_by) {
    order_by.op->accept_visitor(*this);
}

/*
void QueryOptimizer::visit(const OpOptional& optional) {
    optional.op->accept_visitor(*this);
}*/


void QueryOptimizer::visit(const OpLabel&) { }
void QueryOptimizer::visit(const OpProperty&) { }
void QueryOptimizer::visit(const OpConnection&) { }
void QueryOptimizer::visit(const OpConnectionType&) { }
void QueryOptimizer::visit(const OpTransitiveClosure&) { }
void QueryOptimizer::visit(const OpUnjointObject&) { }
