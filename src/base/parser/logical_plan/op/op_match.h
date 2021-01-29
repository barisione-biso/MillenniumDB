#ifndef BASE__OP_MATCH_H_
#define BASE__OP_MATCH_H_

#include <set>
#include <map>
#include <memory>
#include <utility>
#include <vector>

#include "base/parser/logical_plan/exceptions.h"
#include "base/parser/logical_plan/op/op_connection_type.h"
#include "base/parser/logical_plan/op/op_connection.h"
#include "base/parser/logical_plan/op/op_label.h"
#include "base/parser/logical_plan/op/op_property.h"
#include "base/parser/logical_plan/op/op_transitive_closure.h"
#include "base/parser/logical_plan/op/op_unjoint_object.h"
#include "base/parser/logical_plan/op/op.h"

class OpMatch : public Op {
public:
    std::set<OpLabel>             labels;
    std::set<OpProperty>          properties;
    std::set<OpConnection>        connections;
    std::set<OpTransitiveClosure> property_paths; // TODO: for now only supporting transitive closure
    std::set<OpConnectionType>    connection_types;
    std::set<OpUnjointObject>     unjoint_objects;

    std::set<std::string> var_names; // only contains declared variables

    uint_fast32_t* anon_count;

    OpMatch(const std::vector<query::ast::LinearPattern>& graph_pattern, uint_fast32_t* anon_count) :
        anon_count (anon_count)
    {
        for (auto& linear_pattern : graph_pattern) {
            auto last_object_name = process_node(linear_pattern.root);

            if (linear_pattern.path.empty()
                && linear_pattern.root.labels.empty()
                && linear_pattern.root.properties.empty())
            {
                unjoint_objects.insert(OpUnjointObject(last_object_name));
            }

            for (auto& linear_pattern_step : linear_pattern.path) {
                auto current_node_name = process_node(linear_pattern_step.node);

                if (linear_pattern_step.path.type() == typeid(query::ast::Edge)) {
                    // EDGE
                    auto edge = boost::get<query::ast::Edge>(linear_pattern_step.path);
                    auto edge_name  = process_edge(edge);

                    if (edge.direction == query::ast::EdgeDirection::right) {
                        connections.insert(
                            OpConnection(last_object_name, current_node_name, edge_name)
                        );
                    } else {
                        connections.insert(
                            OpConnection(current_node_name, last_object_name, edge_name)
                        );
                    }
                } else {
                    // PROPERTY PATH
                    auto property_path = boost::get<query::ast::PropertyPath>(linear_pattern_step.path);
                    if (property_path.direction == query::ast::EdgeDirection::right) {
                        property_paths.insert(
                            OpTransitiveClosure(last_object_name, current_node_name, property_path.type)
                        );
                    } else {
                        property_paths.insert(
                            OpTransitiveClosure(current_node_name, last_object_name, property_path.type)
                        );
                    }
                }
                last_object_name = std::move(current_node_name);
            }
        }
    }


    std::string process_node(const query::ast::Node& node) {
        std::string node_name;
        if (node.var_or_id.empty()) {
            // anonymous variable
            node_name = "?_" + std::to_string((*anon_count)++);
            var_names.insert(node_name);
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
                    throw QuerySemanticException(node_name + "." + property.key + " its declared with different values");
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
            edge_name = "?_e" + std::to_string((*anon_count)++);
            var_names.insert(edge_name);
        } else if (edge.var_or_id[0] == '?') {
            // explicit variable
            edge_name = edge.var_or_id;
            var_names.insert(edge_name);
        } else {
            // identifier
            edge_name = edge.var_or_id;// (Q1)-[?e :P2 :P5]->(Q3)
                                       // (Q1)-[?e =TYPE(?t)]->(Q3)
                                       // (Q1)-[?e]->(Q3)
        }

        for (const auto& type : edge.types) {
            if (type[0] == '?') {
                var_names.insert(type);
            }
            connection_types.insert(OpConnectionType(edge_name, type));
        }

        if (edge.types.size() == 0) {
            var_names.insert(edge_name + ":type");
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

    std::ostream& print_to_ostream(std::ostream& os, int indent=0) const override{
        os << std::string(indent, ' ');
        os << "OpMatch()\n";

        for (auto& label : labels) {
            label.print_to_ostream(os, indent + 2);
        }
        for (auto& property : properties) {
            property.print_to_ostream(os, indent + 2);
        }
        for (auto& connection : connections) {
            connection.print_to_ostream(os, indent + 2);
        }
        for (auto& property_path : property_paths) {
            property_path.print_to_ostream(os, indent + 2);
        }
        for (auto& connection_type : connection_types) {
            connection_type.print_to_ostream(os, indent + 2);
        }
        for (auto& unjoint_object : unjoint_objects) {
            unjoint_object.print_to_ostream(os, indent + 2);
        }

        return os;
    };

    void accept_visitor(OpVisitor& visitor) override {
        visitor.visit(*this);
    }

    std::set<std::string> get_var_names() const override {
        return var_names;
    }
};

#endif // BASE__OP_MATCH_H_
