#include "bulk_import.h"

#include "base/graph/value/value_string.h"
#include "relational_model/graph/relational_graph.h"

#include <iostream>



BulkImport::BulkImport(const string& nodes_file_name, const string& edges_file_name, RelationalGraph& graph)
    : graph(graph),
      element_labels(OrderedFile("element_labels.dat", 2)),
      element_key_value(OrderedFile("element_key_value.dat", 3)),
      from_to_edge(OrderedFile("from_to_edge.dat", 3))
{
    nodes_file = ifstream(nodes_file_name);
    edges_file = ifstream(edges_file_name);

    node_line_expr = std::regex("^\\s*\\(\\d+\\)(\\s+:[^:\\s]+)*(\\s+[^:\\s]+:((\".+\")|[^\\s\"]+))*\\s*$");
    edge_line_expr = std::regex("^\\s*\\(\\d+\\)->\\(\\d+\\)(\\s+:[^:\\s]+)*(\\s+[^:\\s]+:((\".+\")|[^\\s\"]+))*\\s*$");

    node_expr = std::regex("\\((\\d+)\\)");
    edge_expr = std::regex("\\((\\d+)\\)->\\((\\d+)\\)");

    label_expr = std::regex("\\s+:([^:\\s]+)");
    properties_expr = std::regex("\\s+([^\\s]+):((?:\".*\")|\\S+)");
}

void BulkImport::start_import()
{
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
    cout << "Creating element2label\n";
    vector<uint_fast8_t> element2label_order { 0, 1 };
    element_labels.order(std::move(element2label_order));
    graph.element2label->bulk_import(element_labels);

    cout << "Creating label2element\n";
    vector<uint_fast8_t> label2element_order { 1, 0 };
    element_labels.order(std::move(label2element_order));
    graph.label2element->bulk_import(element_labels);

    cout << "Creating element2prop\n";
    vector<uint_fast8_t> element2prop_order { 0, 1, 2 };
    element_key_value.order(std::move(element2prop_order));
    graph.element2prop->bulk_import(element_key_value);

    cout << "Creating prop2element\n";
    vector<uint_fast8_t> prop2element_order { 0, 1, 2 };
    element_key_value.order(std::move(prop2element_order));
    graph.prop2element->bulk_import(element_key_value);

    cout << "Creating from_to_edge\n";
    vector<uint_fast8_t> from_to_edge_order { 0, 1, 2 };
    from_to_edge.order(std::move(from_to_edge_order));
    graph.from_to_edge->bulk_import(from_to_edge);

    cout << "Creating to_from_edge\n";
    vector<uint_fast8_t> to_from_edge_order { 1, 0, 2 };
    from_to_edge.order(std::move(to_from_edge_order));
    graph.to_from_edge->bulk_import(from_to_edge);
}

void BulkImport::process_node(const string& line, int line_number)
{
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
        element_labels.append_record(graph.get_record_for_node_label(node_id, match[1]));
        str = match.suffix().str();
    }

    // MATCH PROPERTIES
    while (std::regex_search (str, match, properties_expr)) {
        ValueString value = ValueString(match[2]); // TODO: support other types
        element_key_value.append_record(graph.get_record_for_node_property(node_id, match[1], value));
        str = match.suffix().str();
    }
}

void BulkImport::process_edge(const string& line, int line_number)
{
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
        element_labels.append_record(graph.get_record_for_node_label(edge_id, match[1]));
        str = match.suffix().str();
    }

    // MATCH PROPERTIES
    while (std::regex_search (str, match, properties_expr)) {
        ValueString value = ValueString(match[2]); // TODO: support other types
        element_key_value.append_record(graph.get_record_for_node_property(edge_id, match[1], value));
        str = match.suffix().str();
    }
}