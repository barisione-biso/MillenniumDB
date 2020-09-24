#ifndef BASE__OP_MATCH_H_
#define BASE__OP_MATCH_H_

#include <set>
#include <map>
#include <memory>
#include <utility>
#include <vector>

#include "base/parser/logical_plan/exceptions.h"
#include "base/parser/logical_plan/op/op.h"
#include "base/parser/logical_plan/op/op_label.h"
#include "base/parser/logical_plan/op/op_property.h"
#include "base/parser/logical_plan/op/op_connection.h"
#include "base/parser/logical_plan/op/op_connection_type.h"
#include "base/parser/logical_plan/op/op_unjoint_object.h"

class OpMatch : public Op {
public:
    std::set<OpLabel>          labels;
    std::set<OpProperty>       properties;
    std::set<OpConnection>     connections;
    std::set<OpConnectionType> connection_types;
    std::set<OpUnjointObject>  unjoint_objects;

    std::set<std::string> var_names; // only contains declared variables

    int_fast32_t anonymous_var_count = 0;

    OpMatch(const std::vector<query::ast::LinearPattern>& graph_pattern) {
        for (auto& linear_pattern : graph_pattern) {
            auto last_object_name = process_node(linear_pattern.root);

            if (linear_pattern.path.empty()
                && linear_pattern.root.labels.empty()
                && linear_pattern.root.properties.empty())
            {
                unjoint_objects.insert(OpUnjointObject(last_object_name));
            }

            for (auto& step_path : linear_pattern.path) {
                auto current_node_name = process_node(step_path.node);
                auto edge_name         = process_edge(step_path.edge);

                if (step_path.edge.direction == query::ast::EdgeDirection::right) {
                    connections.insert(
                        OpConnection(last_object_name, current_node_name, edge_name)
                    );
                } else {
                    connections.insert(
                        OpConnection(current_node_name, last_object_name, edge_name)
                    );
                }
                last_object_name = std::move(current_node_name);
            }
        }
    }


    std::string process_node(const query::ast::Node& node) {
        std::string node_name;
        if (node.var_or_id.empty()) {
            // anonymous variable
            node_name = "?_" + std::to_string(anonymous_var_count++);
        } else if (node.var_or_id[0] == '?') {
            // explicit variable
            node_name = node.var_or_id;
            var_names.insert(node_name);
        } else {
            // identifier
            node_name = node.var_or_id;
        }

        for (auto& label : node.labels) {
            labels.insert(OpLabel(node_name, label));
        }

        for (auto& property : node.properties) {
            auto new_property = OpProperty(node_name, property.key, property.value);
            auto property_search = properties.find(new_property);

            if (property_search != properties.end()) {
                auto old_property = *property_search;
                if (old_property.value != property.value) {
                    throw QuerySemanticException(node_name + "." + property.key + " its declared with different values.");
                }
            } else {
                properties.insert(new_property);
            }
        }
        return node_name;
    }


    std::string process_edge(const query::ast::Edge& edge) {
        std::string edge_name;
        if (edge.var_or_id.empty()) {
            // anonymous variable
            edge_name = "?_e" + std::to_string(anonymous_var_count++);
        } else if (edge.var_or_id[0] == '?') {
            // explicit variable
            edge_name = edge.var_or_id;
            var_names.insert(edge_name);
        } else {
            // identifier
            edge_name = edge.var_or_id;
        }

        for (auto& type : edge.types) {
            connection_types.insert(OpConnectionType(edge_name, type));
        }

        for (auto& property : edge.properties) {
            auto new_property = OpProperty(edge_name, property.key, property.value);
            auto property_search = properties.find(new_property);

            if (property_search != properties.end()) {
                auto old_property = *property_search;
                if (old_property.value != property.value) {
                    throw QuerySemanticException(edge_name + "." + property.key + " its declared with different values in MATCH");
                }
            } else {
                properties.insert(new_property);
            }
        }

        return edge_name;
    }


    void accept_visitor(OpVisitor& visitor) {
        visitor.visit(*this);
    }
};

#endif // BASE__OP_MATCH_H_
