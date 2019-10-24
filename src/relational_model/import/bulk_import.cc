#include "bulk_import.h"

#include "base/graph/value/value_string.h"
#include "relational_model/graph/relational_graph.h"

#include <iostream>
#include <regex>


BulkImport::BulkImport(const string& nodes_file_name, const string& edges_file_name, RelationalGraph& graph)
    : graph(graph)
{
    nodes_file = ifstream(nodes_file_name);
    edges_file = ifstream(edges_file_name);
}

void BulkImport::start_import()
{
    node_count = 0;
    edge_count = 0;
    string line;

    while (getline(nodes_file, line)) {
        process_node(line);
    }

    while (getline(edges_file, line)) {
        process_edge(line);
    }

    // connect nodes
    for (auto&& [from, to, edge_id] : edges_original_ids) {
        graph.connect_nodes(node_dict[from], node_dict[to], edge_id);
    }
}

void BulkImport::process_node(const string& line)
{
    std::regex line_expr ("^\\s*\\(\\d+\\)(\\s+:\\w+)*(\\s+\\w+:((\"(\\w|\\s)*\")|\\S+))*\\s*$");
    std::smatch match;

    std::regex_search(line, match, line_expr);
    if (match.empty()) {
        cout << "ERROR: line has wrong format in nodes file.\n";
        return;
    }

    std::string str = line;

    std::regex node_expr ("\\((\\d+)\\)");

    // MATCH NODE ID
    std::regex_search(str, match, node_expr);
    uint64_t original_id = stoul(match[1]);
    uint64_t node_id = graph.create_node();
    str = match.suffix().str();

    node_count++;
    // TODO: check threshold to begin disk mode
    node_dict.insert(pair<uint64_t, uint64_t>(original_id, node_id));

    // MATCH LABELS
    std::regex label_expr ("\\s+:(\\w+)");
    while (std::regex_search (str, match, label_expr)) {
        graph.add_label_to_node(node_id, match[1]);
        str = match.suffix().str();
    }

    // MATCH PROPERTIES
    std::regex properties_expr ("\\s+(\\w+):((?:\"(?:\\w|\\s)*\")|\\S+)");
    while (std::regex_search (str, match, properties_expr)) {
        ValueString value = ValueString(match[2]); // TODO: support other types
        graph.add_property_to_node(node_id, match[1], value);
        str = match.suffix().str();
    }
}

void BulkImport::process_edge(const string& line)
{

    // ^\s*\(\d+\)->\(\d+\)(\s+:(\w+))*(\s+\w+:((\"(\w|\s)*\")|\S+))*\s*$
    std::regex line_expr ("^\\s*\\(\\d+\\)->\\(\\d+\\)(\\s+:(\\w+))*(\\s+\\w+:((\"(\\w|\\s)*\")|\\S+))*\\s*$");
    std::smatch match;

    std::regex_search(line, match, line_expr);
    if (match.empty()) {
        cout << "ERROR: line has wrong format in edges filw.\n";
        return;
    }

    std::string str = line;

    std::regex node_expr ("\\((\\d+)\\)->\\((\\d+)\\)");

    // MATCH NODES IDS
    std::regex_search(str, match, node_expr);
    uint64_t original_id_from = stoul(match[1]);
    uint64_t original_id_to = stoul(match[2]);

    uint64_t edge_id = graph.create_edge();
    edge_count++;
    // TODO: check threshold to begin disk mode
    edges_original_ids.push_back(tuple<uint64_t, uint64_t, uint64_t>(original_id_from, original_id_to, edge_id));
    str = match.suffix().str();

    // MATCH LABELS
    std::regex label_expr ("\\s+:(\\w+)");
    while (std::regex_search (str, match, label_expr)) {
        graph.add_label_to_edge(edge_id, match[1]);
        str = match.suffix().str();
    }

    // MATCH PROPERTIES
    std::regex properties_expr ("\\s+(\\w+):((?:\"(?:\\w|\\s)*\")|\\S+)");
    while (std::regex_search (str, match, properties_expr)) {
        ValueString value = ValueString(match[2]); // TODO: support other types
        graph.add_property_to_edge(edge_id, match[1], value);
        str = match.suffix().str();
    }
}
