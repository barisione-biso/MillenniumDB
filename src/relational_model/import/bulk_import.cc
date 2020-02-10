#include "bulk_import.h"

#include "base/graph/value/value_string.h"
#include "relational_model/graph/relational_graph.h"

#include <iostream>


BulkImport::BulkImport(const string& nodes_file_name, const string& edges_file_name, RelationalGraph& graph)
    : graph(graph),
      node_labels(OrderedFile("node_labels.dat", 2)),
      edge_labels(OrderedFile("edge_labels.dat", 2)),
      node_key_value(OrderedFile("node_key_value.dat", 3)),
      edge_key_value(OrderedFile("edge_key_value.dat", 3)),
      from_to_edge(OrderedFile("from_to_edge.dat", 3))
{
    nodes_file = ifstream(nodes_file_name);
    edges_file = ifstream(edges_file_name);

    if (nodes_file.fail()) {
        std::cerr << "Nodes file: \"" << nodes_file_name << "\" could not be opened. Exiting.\n";
        exit(-1);

    } else if (edges_file.fail()){
        std::cerr << "Edges file: \"" << edges_file_name << "\" could not be opened. Exiting.\n";
        exit(-1);
    }


    node_line_expr = std::regex("^\\s*\\(\\d+\\)(\\s+:[^:\\s]+)*(\\s+[^:\\s]+:((\".+\")|[^\\s\"]+))*\\s*$");
    edge_line_expr = std::regex("^\\s*\\(\\d+\\)->\\(\\d+\\)(\\s+:[^:\\s]+)*(\\s+[^:\\s]+:((\".+\")|[^\\s\"]+))*\\s*$");

    node_expr = std::regex("\\((\\d+)\\)");
    edge_expr = std::regex("\\((\\d+)\\)->\\((\\d+)\\)");

    label_expr = std::regex("\\s+:([^:\\s]+)");
    properties_expr = std::regex("\\s+([^\\s]+):((?:\".*\")|\\S+)");
}


void BulkImport::start_import() {
    node_count = 0;
    edge_count = 0;
    string line;

    int line_number = 1;
    cout << "procesing nodes:\n";
    while (getline(nodes_file, line)) {
        process_node(line, line_number);
        line_number++;
    }
    cout << "\n";

    line_number = 1;
    cout << "procesing edges:\n";
    while (getline(edges_file, line)) {
        process_edge(line, line_number);
        line_number++;
    }
    cout << "\n";

    // connect nodes
    line_number = 1;
    cout << "conecting nodes:\n";
    for (auto&& [from, to, edge_id] : edges_original_ids) {
        cout << "\r  line " << line_number++ << std::flush;
        from_to_edge.append_record(Record(node_dict[from], node_dict[to], edge_id));
    }
    cout << "\nCreating indexes for labels\n";
    // NODE LABELS
    node_labels.order(vector<uint_fast8_t> { 0, 1 });
    graph.node2label->bulk_import(node_labels);

    node_labels.order(vector<uint_fast8_t> { 1, 0 });
    graph.label2node->bulk_import(node_labels);

    // EDGE LABELS
    edge_labels.order(vector<uint_fast8_t> { 0, 1 });
    graph.edge2label->bulk_import(edge_labels);

    edge_labels.order(vector<uint_fast8_t> { 1, 0 });
    graph.label2edge->bulk_import(edge_labels);

    cout << "Creating indexes for properties\n";

    // NODE PROPERTIES
    node_key_value.order(vector<uint_fast8_t> { 0, 1, 2 });
    graph.node2prop->bulk_import(node_key_value);

    node_key_value.order(vector<uint_fast8_t> { 2, 0, 1 });
    graph.prop2node->bulk_import(node_key_value);

    // EDGE PROPERTIES
    edge_key_value.order(vector<uint_fast8_t> { 0, 1, 2 });
    graph.edge2prop->bulk_import(edge_key_value);

    edge_key_value.order(vector<uint_fast8_t> { 2, 0, 1 });
    graph.prop2edge->bulk_import(edge_key_value);

    cout << "Creating indexes for connections\n";

    // CONNECTIONS
    from_to_edge.order(vector<uint_fast8_t> { 0, 1, 2 });
    graph.from_to_edge->bulk_import(from_to_edge);

    from_to_edge.order(vector<uint_fast8_t> { 2, 0, 1 }); // TODO: revisar
    graph.to_edge_from->bulk_import(from_to_edge);

    from_to_edge.order(vector<uint_fast8_t> { 2, 0, 1 }); // TODO: revisar
    graph.edge_from_to->bulk_import(from_to_edge);
}


void BulkImport::process_node(const string& line, int line_number) {
    std::smatch match;
    cout << "\r  line " << line_number << std::flush;

    std::regex_search(line, match, node_line_expr);
    if (match.empty()) {
        cout << "ERROR: line " << line_number << " has wrong format in nodes file.\n";
        return;
    }
    std::string str = line;

    // MATCH NODE ID
    std::regex_search(str, match, node_expr);
    uint64_t original_id = stoul(match[1]);
    uint64_t node_id = graph.create_node();
    str = match.suffix().str();

    node_count++;
    // TODO: check threshold to begin disk mode
    node_dict.insert(pair<uint64_t, uint64_t>(original_id, node_id));

    // MATCH LABELS
    while (std::regex_search (str, match, label_expr)) {
        node_labels.append_record(graph.get_record_for_node_label(node_id, match[1]));
        str = match.suffix().str();
    }

    // MATCH PROPERTIES
    while (std::regex_search (str, match, properties_expr)) {
        ValueString value = ValueString(match[2]); // TODO: support other types
        node_key_value.append_record(graph.get_record_for_node_property(node_id, match[1], value));
        str = match.suffix().str();
    }
}


void BulkImport::process_edge(const string& line, int line_number) {
    std::smatch match;
    cout << "\r  line " << line_number << std::flush;

    std::regex_search(line, match, edge_line_expr);
    if (match.empty()) {
         cout << "ERROR: line " << line_number << " has wrong format in edges file.\n";
        return;
    }

    std::string str = line;

    // MATCH NODES IDS
    std::regex_search(str, match, edge_expr);
    uint64_t original_id_from = stoul(match[1]);
    uint64_t original_id_to = stoul(match[2]);

    uint64_t edge_id = graph.create_edge();
    edge_count++;
    // TODO: check threshold to begin disk mode
    edges_original_ids.push_back(tuple<uint64_t, uint64_t, uint64_t>(original_id_from, original_id_to, edge_id));
    str = match.suffix().str();

    // MATCH LABELS
    while (std::regex_search (str, match, label_expr)) {
        edge_labels.append_record(graph.get_record_for_edge_label(edge_id, match[1]));
        str = match.suffix().str();
    }

    // MATCH PROPERTIES
    while (std::regex_search (str, match, properties_expr)) {
        ValueString value = ValueString(match[2]); // TODO: support other types
        edge_key_value.append_record(graph.get_record_for_edge_property(edge_id, match[1], value));
        str = match.suffix().str();
    }
}
