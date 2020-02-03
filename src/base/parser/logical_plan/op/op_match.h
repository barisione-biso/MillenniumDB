#ifndef BASE__OP_MATCH_H_
#define BASE__OP_MATCH_H_

#include "base/parser/logical_plan/op/op.h"
#include "base/parser/logical_plan/op/op_label.h"
#include "base/parser/logical_plan/op/op_property.h"
#include "base/parser/logical_plan/op/op_connection.h"

#include <set>
#include <variant>
#include <vector>

class OpMatch : public Op {
public:
    std::vector< std::variant<OpLabel, OpProperty, OpConnection> > op_list;

    std::set<std::string> nodes;
    std::set<std::string> edges;
    int_fast32_t anonymous_var_count = 0;


    OpMatch(const std::vector<ast::LinearPattern>& graph_pattern) {
        for (auto& linear_pattern : graph_pattern) {
            auto last_node_name = process_node(linear_pattern.root);

            for (auto& step_path : linear_pattern.path) {
                auto current_node_name = process_node(step_path.node);
                auto edge_name         = process_edge(step_path.edge);

                if (step_path.edge.direction == ast::EdgeDirection::right) {
                    op_list.push_back(OpConnection(last_node_name, edge_name, current_node_name));
                }
                else {
                    op_list.push_back(OpConnection(current_node_name, edge_name, last_node_name));
                }
                last_node_name = std::move(current_node_name);
            }
        }
    }


    std::string process_node(const ast::Node& node) {
        std::string var_name;
        if (node.var.empty()) {
            var_name = "_" + (anonymous_var_count++);
        }
        else {
            var_name = node.var;

            // check no edge has the same name
            if (edges.find(var_name) != edges.end()) {
                throw ParsingException();
            }

            auto found = nodes.find(var_name);
            if (found == nodes.end()) { // not found
                nodes.insert(var_name);
            }
        }

        for (auto& label : node.labels) {
            op_list.push_back(OpLabel(ElementType::node, var_name, label));
        }

        for (auto& property : node.properties) {
            op_list.push_back(OpProperty(ElementType::node, var_name, property.key, property.value));
        }

        return var_name;
    }


    std::string process_edge(const ast::Edge& edge) {
        std::string var_name;
        if (edge.var.empty()) {
            var_name = "_" + (anonymous_var_count++);
        }
        else {
            var_name = edge.var;

            // check no node has the same name
            if (nodes.find(var_name) != nodes.end()) {
                throw ParsingException();
            }

            auto found = edges.find(var_name);
            if (found == edges.end()) { // not found
                edges.insert(var_name);
            }
        }

        for (auto& label : edge.labels) {
            op_list.push_back(OpLabel(ElementType::edge, var_name, label));
        }

        for (auto& property : edge.properties) {
            op_list.push_back(OpProperty(ElementType::edge, var_name, property.key, property.value));
        }

        return var_name;
    }


    void visit(OpVisitor& visitor) {
        visitor.visit(*this);
    }
};

#endif //BASE__OP_MATCH_H_
