#ifndef BASE__OP_MATCH_H_
#define BASE__OP_MATCH_H_

#include <set>
#include <map>
#include <memory>
#include <utility>
#include <variant>
#include <vector>

#include "base/parser/logical_plan/exceptions.h"
#include "base/parser/logical_plan/op/op.h"
#include "base/parser/logical_plan/op/op_node_label.h"
#include "base/parser/logical_plan/op/op_node_property.h"
#include "base/parser/logical_plan/op/op_edge_label.h"
#include "base/parser/logical_plan/op/op_edge_property.h"
#include "base/parser/logical_plan/op/op_connection.h"
#include "base/parser/logical_plan/op/op_node_loop.h"
#include "base/parser/logical_plan/op/op_lonely_node.h"

class OpMatch : public Op {
public:
    std::set<OpNodeLabel>    node_labels;
    std::set<OpNodeProperty> node_properties;
    std::set<OpEdgeLabel>    edge_labels;
    std::set<OpEdgeProperty> edge_properties;
    std::set<OpConnection>   connections;
    std::set<OpLonelyNode>   lonely_nodes;
    std::set<OpNodeLoop>     node_loops;

    std::set<std::string>    node_names;
    std::set<std::string>    edge_names;
    std::map<std::string, std::string> var_name2graph_name;

    int_fast32_t anonymous_var_count = 0;

    OpMatch(const std::vector<ast::LinearPattern>& graph_pattern) {
        for (auto& linear_pattern : graph_pattern) {
            auto graph_name = linear_pattern.graph_name;
            auto last_node_name = process_node(graph_name, linear_pattern.root);

            if (linear_pattern.path.empty()
                && linear_pattern.root.labels.empty()
                && linear_pattern.root.properties.empty())
            {
                lonely_nodes.insert(OpLonelyNode(graph_name, last_node_name));
            }

            for (auto& step_path : linear_pattern.path) {
                auto current_node_name = process_node(graph_name, step_path.node);
                auto edge_name         = process_edge(graph_name, step_path.edge);

                if (last_node_name == current_node_name) {
                    node_loops.insert(
                        OpNodeLoop(graph_name, last_node_name, edge_name)
                    );
                } else if (step_path.edge.direction == ast::EdgeDirection::right) {
                    connections.insert(
                        OpConnection(graph_name, last_node_name, edge_name, current_node_name)
                    );
                } else {
                    connections.insert(
                        OpConnection(graph_name, current_node_name, edge_name, last_node_name)
                    );
                }
                last_node_name = std::move(current_node_name);
            }
        }
    }


    std::string process_node(const std::string& graph_name, const ast::Node& node) {
        std::string node_name;
        if (node.var.empty()) {
            node_name = "_n" + std::to_string(anonymous_var_count++);
        } else {
            node_name = node.var;
            auto search = edge_names.find(node_name);

            // check no edge has same name
            if (search != edge_names.end()) {
                throw QuerySemanticException("\"" + node_name
                    + "\" has already been declared as an Edge and cannot be a Node");
            }
            // check graph name is the same
            if (var_name2graph_name[node_name] != graph_name) {
                throw QuerySemanticException("\"" + node_name + "\" has already been declared in graph '"
                    + var_name2graph_name[node_name]
                    + "' and cannot be declared in another graph (" + graph_name + ")");
            }
        }
        node_names.insert(node_name);
        var_name2graph_name.insert({ node_name, graph_name });

        for (auto& label : node.labels) {
            node_labels.insert(OpNodeLabel(graph_name, node_name, label));
        }

        for (auto& property : node.properties) {
            auto new_property = OpNodeProperty(graph_name, node_name, property.key, property.value);
            auto property_search = node_properties.find(new_property);

            if (property_search != node_properties.end()) {
                auto old_property = *property_search;
                if (old_property.value != property.value) {
                    throw QuerySemanticException(node_name + "." + property.key + " its declared with different values.");
                }
            } else {
                node_properties.insert(new_property);
            }
        }
        return node_name;
    }


    std::string process_edge(const std::string& graph_name, const ast::Edge& edge) {
        std::string edge_name;
        if (edge.var.empty()) {
            edge_name = "_e" + std::to_string(anonymous_var_count++);
        } else {
            edge_name = edge.var;

            auto search = node_names.find(edge_name);

            // check no node has same name
            if (search != edge_names.end()) {
                throw QuerySemanticException("\"" + edge_name
                    + "\" has already been declared as an Node and cannot be a Edge");
            }
            // check graph name is the same
            if (var_name2graph_name[edge_name] != graph_name) {
                throw QuerySemanticException("\"" + edge_name + "\" has already been declared in graph '"
                    + var_name2graph_name[edge_name]
                    + "' and cannot be declared in another graph (" + graph_name + ")");
            }
        }
        edge_names.insert(edge_name);
        var_name2graph_name.insert({ edge_name, graph_name });

        for (auto& label : edge.labels) {
            edge_labels.insert(OpEdgeLabel(graph_name, edge_name, label));
        }

        for (auto& property : edge.properties) {
            auto new_property = OpEdgeProperty(graph_name, edge_name, property.key, property.value);
            auto property_search = edge_properties.find(new_property);

            if (property_search != edge_properties.end()) {
                auto old_property = *property_search;
                if (old_property.value != property.value) {
                    throw QuerySemanticException(edge_name + "." + property.key + " its declared with different values in MATCH");
                }
            } else {
                edge_properties.insert(new_property);
            }
        }

        return edge_name;
    }


    void accept_visitor(OpVisitor& visitor) {
        visitor.visit(*this);
    }
};

#endif // BASE__OP_MATCH_H_
