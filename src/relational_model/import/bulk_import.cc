#include "bulk_import.h"

#include "base/graph/value/value_string.h"
#include "relational_model/import/bulk_import_grammar.h"
#include "relational_model/import/bulk_import_value_visitor.h"
#include "relational_model/graph/relational_graph.h"

#include <iostream>
#include <boost/spirit/include/support_istream_iterator.hpp>

using namespace std;

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

    nodes_file.unsetf(std::ios::skipws);
    edges_file.unsetf(std::ios::skipws);

    if (nodes_file.fail()) {
        std::cerr << "Nodes file: \"" << nodes_file_name << "\" could not be opened. Exiting.\n";
        exit(-1);

    } else if (edges_file.fail()){
        std::cerr << "Edges file: \"" << edges_file_name << "\" could not be opened. Exiting.\n";
        exit(-1);
    }
}


void BulkImport::start_import() {
    int line_number = 1;
    cout << "procesing nodes:\n";

    boost::spirit::istream_iterator node_iter( nodes_file );
    boost::spirit::istream_iterator node_end;
    do {
        bulk_import_ast::Node node;
        bool r = phrase_parse(node_iter, node_end, bulk_import_parser::node, bulk_import_parser::skipper, node);
        if (r) {
            cout << "\r  line " << line_number << std::flush;
            process_node(node);
            line_number++;
        }
        else {
            cout << "ERROR: line " << line_number << " has wrong format in nodes file.\n";
            return;
        }
    } while(node_iter != node_end);
    cout << "\n";

    line_number = 1;
    cout << "procesing edges:\n";

    boost::spirit::istream_iterator edge_iter( edges_file );
    boost::spirit::istream_iterator edge_end;
    do {
        bulk_import_ast::Edge edge;
        bool r = phrase_parse(edge_iter, edge_end, bulk_import_parser::edge, bulk_import_parser::skipper, edge);
        if (r) {
            cout << "\r  line " << line_number << std::flush;
            process_edge(edge);
            line_number++;
        }
        else {
            cout << "ERROR: line " << line_number << " has wrong format in edges file.\n";
            return;
        }
    } while(edge_iter != edge_end);
    cout << "\n";

    // connect nodes
    line_number = 1;
    cout << "connecting nodes:\n";
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

    from_to_edge.order(vector<uint_fast8_t> { 2, 0, 1 });
    graph.to_edge_from->bulk_import(from_to_edge);

    from_to_edge.order(vector<uint_fast8_t> { 2, 0, 1 });
    graph.edge_from_to->bulk_import(from_to_edge);
}


void BulkImport::process_node(const bulk_import_ast::Node& node) {
    uint64_t node_id = graph.create_node();

    // TODO: check threshold to begin disk mode
    node_dict.insert(pair<uint64_t, uint64_t>(node.id, node_id));

    for (auto& label : node.labels) {
        node_labels.append_record(graph.get_record_for_node_label(node_id, label));
    }

    for (auto& property : node.properties) {
        BulkImportValueVisitor visitor;
        auto value = visitor(property.value);
        node_key_value.append_record(graph.get_record_for_node_property(node_id, property.key, *value));
    }
}


void BulkImport::process_edge(const bulk_import_ast::Edge& edge) {
    uint64_t edge_id = graph.create_edge();

    // TODO: check threshold to begin disk mode
    if (edge.direction == bulk_import_ast::EdgeDirection::right) {
        edges_original_ids.push_back(tuple<uint64_t, uint64_t, uint64_t>(edge.left_id, edge.right_id, edge_id));
    }
    else {
        edges_original_ids.push_back(tuple<uint64_t, uint64_t, uint64_t>(edge.right_id, edge.left_id, edge_id));
    }

    for (auto& label : edge.labels) {
        edge_labels.append_record(graph.get_record_for_edge_label(edge_id, label));
    }

    for (auto& property : edge.properties) {
        BulkImportValueVisitor visitor;
        auto value = visitor(property.value);
        edge_key_value.append_record(graph.get_record_for_edge_property(edge_id, property.key, *value));
    }
}
