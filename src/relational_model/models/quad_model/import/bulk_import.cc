#include "bulk_import.h"

#include <chrono>
#include <iostream>
#include <boost/spirit/include/support_istream_iterator.hpp>

#include "base/graph/value/value_string.h"
#include "base/parser/grammar/import/import_ast.h"
#include "base/parser/grammar/import/import_ast_adapted.h"
#include "base/parser/grammar/import/import.h"
#include "base/parser/grammar/import/import_def.h"
#include "base/parser/grammar/common/value_visitor.h"
#include "storage/index/ordered_file/bpt_merger.h"
#include "storage/buffer_manager.h"
#include "storage/file_manager.h"

using namespace std;

BulkImport::BulkImport(const string& filename, QuadModel& model) :
    model                (model),
    catalog              (model.catalog()),
    node_labels          (OrderedFile<2>("node_labels.dat")),
    object_key_value     (OrderedFile<3>("object_key_value.dat")),
    from_to_type_edge    (OrderedFile<4>("from_to_type_edge.dat"))
    // equal_from_to        (OrderedFile<3>("equal_from_to.dat")),
    // equal_from_type      (OrderedFile<3>("equal_from_type.dat")),
    // equal_to_type        (OrderedFile<3>("equal_to_type.dat")),
    // equal_from_to_type   (OrderedFile<2>("equal_from_to_type.dat")),
{
    import_file = ifstream(filename);
    import_file.unsetf(std::ios::skipws);

    if (import_file.fail()) {
        std::cerr << "File: \"" << filename << "\" could not be opened. Exiting.\n";
        exit(-1);
    }
}


void BulkImport::start_import() {
    auto start = chrono::system_clock::now();
    auto line_number = 1;
    std::cout << "Reading file:\n";

    boost::spirit::istream_iterator file_iter( import_file );
    boost::spirit::istream_iterator file_iter_end;

    vector<uint64_t> ids_stack;

    do {
        import::ast::ImportLine import_line;
        bool r = phrase_parse(file_iter, file_iter_end, import::import(), import::parser::skipper, import_line);
        if (r) {
            if (import_line.type() == typeid(import::ast::Node)) {
                auto node_id = process_node( boost::get<import::ast::Node>(import_line) );
                ids_stack.clear();
                ids_stack.push_back(node_id);
            }
            else if (import_line.type() == typeid(import::ast::Edge)) {
                auto edge_id = process_edge( boost::get<import::ast::Edge>(import_line) );
                ids_stack.clear();
                ids_stack.push_back(edge_id);
            }
            else if (import_line.type() == typeid(import::ast::ImplicitEdge)) {
                auto implicit_edge = boost::get<import::ast::ImplicitEdge>(import_line);
                const auto nesting_lvl = implicit_edge.nesting_level();

                if (ids_stack.size() == 0) {
                    throw logic_error("ERROR on line " + std::to_string(line_number)
                        + ": can't use implicit edge on undefined object");
                }
                else if (nesting_lvl < ids_stack.size()) {
                    auto edge_id = process_implicit_edge(implicit_edge, ids_stack[nesting_lvl-1] );
                    ids_stack.resize(nesting_lvl);
                    ids_stack.push_back(edge_id);
                }
                else if (nesting_lvl == ids_stack.size()) {
                    auto edge_id = process_implicit_edge(implicit_edge, ids_stack[nesting_lvl-1] );
                    ids_stack.push_back(edge_id);
                }
                else {
                    throw logic_error("ERROR on line " + std::to_string(line_number)
                        + ": undefined level of implicit edge");
                }
            }

            line_number++;
        } else {
            cerr << "ERROR: line " << line_number << " has wrong format in import file.\n";
            // TODO: print line (text)?
            // string context(file_iter, file_iter.);
            // cerr << "----------------------------\n";
            // cerr << " Parsing failed, stopped at:\n" << context << "\n";
            // cerr << "----------------------------\n";
            return;
        }
    } while(file_iter != file_iter_end);

    auto finish_reading_files = chrono::system_clock::now();
    chrono::duration<float, milli> phase1_duration = finish_reading_files - start;
    std::cout << "Reading files & writing ordered file: " << phase1_duration.count() << "ms\n";

    std::cout << "\nCreating indexes\n";

    // NODE - LABEL
    node_labels.order(std::array<uint_fast8_t, 2> { 0, 1 });
    model.node_label->bulk_import(node_labels);

    // LABEL - NODE
    node_labels.order(std::array<uint_fast8_t, 2> { 1, 0 });
    model.label_node->bulk_import(node_labels);

    // OBJECT - KEY - VALUE
    object_key_value.order(std::array<uint_fast8_t, 3> { 0, 1, 2 });
    model.object_key_value->bulk_import(object_key_value);

    // KEY - VALUE - OBJECT
    object_key_value.order(std::array<uint_fast8_t, 3> { 2, 0, 1 });
    model.key_value_object->bulk_import(object_key_value);

    // CONNECTIONS
    from_to_type_edge.order(std::array<uint_fast8_t, 4> { 0, 1, 2, 3 });
    model.from_to_type_edge->bulk_import(from_to_type_edge);

    from_to_type_edge.order(std::array<uint_fast8_t, 4> { 2, 0, 1, 3 });
    model.to_type_from_edge->bulk_import(from_to_type_edge);

    from_to_type_edge.order(std::array<uint_fast8_t, 4> { 2, 0, 1, 3 });
    model.type_from_to_edge->bulk_import(from_to_type_edge);

    // SPECIAL CASES
    // TODO:
    // equal_from_to.order(std::array<uint_fast8_t, 3> { 0, 1, 2 });
    // model.equal_from_to->bulk_import(equal_from_to);

    // equal_from_type.order(std::array<uint_fast8_t, 3> { 0, 1, 2 });
    // model.equal_from_type->bulk_import(equal_from_type);

    // equal_to_type.order(std::array<uint_fast8_t, 3> { 0, 1, 2 });
    // model.equal_to_type->bulk_import(equal_to_type);

    // equal_from_to_type.order(std::array<uint_fast8_t, 3> { 0, 1, 2 });
    // model.equal_from_to_type->bulk_import(equal_from_to_type);

    catalog.distinct_labels = catalog.label2total_count.size();
    catalog.distinct_types  = catalog.type2total_count.size();
    catalog.distinct_keys   = catalog.key2total_count.size();
    // TODO: set catalog.key2distinct

    auto finish_creating_index = chrono::system_clock::now();
    chrono::duration<float, milli> phase2_duration = finish_creating_index - finish_reading_files;
    std::cout << "Writing indexes: " << phase2_duration.count() << "ms\n";

    if (catalog.identifiable_defined_count != catalog.identifiable_nodes_count) {
        std::cout << "WARNING: defined identifiable nodes(" << catalog.identifiable_defined_count
        << ") and used identifiable nodes (" << catalog.identifiable_nodes_count << ") are different.\n";
    }

    if (catalog.anonymous_defined_count != catalog.anonymous_nodes_count) {
        std::cout << "WARNING: defined anonymous nodes(" << catalog.anonymous_defined_count
        << ") and used anonymous nodes (" << catalog.anonymous_nodes_count << ") are different.\n";
    }

    catalog.save_changes();
}


// template <std::size_t N>
// void BulkImport::merge_tree_and_ordered_file(unique_ptr<BPlusTree<N>>& bpt, OrderedFile<N>& ordered_file)
// {
//     auto original_dir_filename  = file_manager.get_filename(bpt->dir_file_id);
//     auto original_leaf_filename = file_manager.get_filename(bpt->leaf_file_id);
//     auto original_filename = original_dir_filename.substr(0, original_dir_filename.size()-4);
//     auto tmp_filename = original_filename + ".tmp";

//     auto new_bpt = make_unique<BPlusTree<N>>(tmp_filename);
//     { // new scope so bpt_merger is destroyed before file_manager.remove
//         auto bpt_merger = BptMerger<N>(ordered_file, *bpt);
//         new_bpt->bulk_import(bpt_merger);
//     }

//     auto old_dir_file_id  = bpt->dir_file_id;
//     auto old_leaf_file_id = bpt->leaf_file_id;

//     auto new_dir_file_id  = new_bpt->dir_file_id;
//     auto new_leaf_file_id = new_bpt->leaf_file_id;

//     bpt = move(new_bpt);

//     file_manager.remove(old_dir_file_id);
//     file_manager.remove(old_leaf_file_id);

//     file_manager.rename(new_dir_file_id,  original_dir_filename);
//     file_manager.rename(new_leaf_file_id, original_leaf_filename);
// }
uint64_t BulkImport::get_node_id(const string& node_name) {
    auto search = identificable_node_dict.find(node_name);
    if (search != identificable_node_dict.end()) {
        return search->second;
    } else {
        auto new_id = model.get_string_id(node_name, true).id;
        model.node_table->append_record(RecordFactory::get(new_id));

        identificable_node_dict.insert({ node_name, new_id });
        ++catalog.identifiable_nodes_count;
        return new_id;
    }
}


uint64_t BulkImport::get_anonymous_node_id(const string& tmp_name) {
    auto search = anonymous_node_dict.find(tmp_name);
    if (search != anonymous_node_dict.end()) {
        return search->second;
    } else {
        auto new_anonymous_id = (++catalog.anonymous_nodes_count) | model.ANONYMOUS_NODE_MASK;
        anonymous_node_dict.insert({ tmp_name, new_anonymous_id });
        return new_anonymous_id;
    }
}


uint64_t BulkImport::process_node(const import::ast::Node node) {
    uint64_t node_id;
    if (node.anonymous()) {
        node_id = get_anonymous_node_id(node.name);
        ++catalog.anonymous_defined_count;
    } else {
        node_id = get_node_id(node.name);
        ++catalog.identifiable_defined_count;
    }

    for (auto& label : node.labels) {
        auto label_id = model.get_string_id(label, true).id;
        ++catalog.label_count;
        ++catalog.label2total_count[label_id]; // TODO: funciona?

        node_labels.append_record(RecordFactory::get(node_id, label_id));
    }

    for (auto& property : node.properties) {
        ValueVisitor visitor;
        auto value = visitor(property.value);

        auto key_id   = model.get_string_id(property.key, true).id;
        auto value_id = model.get_value_id(*value, true).id;

        ++catalog.properties_count;
        ++catalog.key2total_count[key_id]; // TODO: funciona?

        object_key_value.append_record(RecordFactory::get(node_id, key_id, value_id));
    }
    return node_id;
}


uint64_t BulkImport::process_edge(const import::ast::Edge edge) {
    uint64_t left_id;
    uint64_t right_id;
    uint64_t type_id;

    if (edge.labels.size() == 1) {
        type_id = get_node_id(edge.labels[0]);
    } else {
        throw logic_error("In this quad bulk import all edges should have 1 type");
    }

    if (edge.left_anonymous()) {
        left_id = get_anonymous_node_id(edge.left_name);
    } else {
        left_id = get_node_id(edge.left_name);
    }

    if (edge.right_anonymous()) {
        right_id = get_anonymous_node_id(edge.right_name);
    } else {
        right_id = get_node_id(edge.right_name);
    }

    uint64_t edge_id;
    if (edge.direction == import::ast::EdgeDirection::right) {
        edge_id = create_connection(left_id, right_id, type_id);
    } else {
        edge_id = create_connection(right_id, left_id, type_id);
    }

    for (auto& property : edge.properties) {
        ValueVisitor visitor;
        auto value = visitor(property.value);

        auto key_id   = model.get_string_id(property.key, true).id;
        auto value_id = model.get_value_id(*value, true).id;

        ++catalog.properties_count;
        ++catalog.key2total_count[key_id]; // TODO: funciona?

        object_key_value.append_record(RecordFactory::get(edge_id, key_id, value_id));
    }
    ++catalog.type2total_count[type_id];   // TODO: funciona?
    return edge_id;
}


uint64_t BulkImport::process_implicit_edge(const import::ast::ImplicitEdge edge,
                                           const uint64_t implicit_object_id)
{
    uint64_t right_id;
    uint64_t type_id;

    if (edge.labels.size() == 1) {
        type_id = get_node_id(edge.labels[0]);
    } else {
        throw logic_error("In this quad bulk import all edges should have 1 type");
    }


    if (edge.right_anonymous()) {
        right_id = get_anonymous_node_id(edge.right_name);
    } else {
        right_id = get_node_id(edge.right_name);
    }

    uint64_t edge_id;
    if (edge.direction == import::ast::EdgeDirection::right) {
        edge_id = create_connection(implicit_object_id, right_id, type_id);
    } else {
        edge_id = create_connection(right_id, implicit_object_id, type_id);
    }

    for (auto& property : edge.properties) {
        ValueVisitor visitor;
        auto value = visitor(property.value);

        auto key_id   = model.get_string_id(property.key, true).id;
        auto value_id = model.get_value_id(*value, true).id;

        ++catalog.properties_count;
        ++catalog.key2total_count[key_id]; // TODO: funciona?

        object_key_value.append_record(RecordFactory::get(edge_id, key_id, value_id));
    }
    ++catalog.type2total_count[type_id];   // TODO: funciona?
    return edge_id;
}


uint64_t BulkImport::create_connection(const uint64_t from_id, const uint64_t to_id, const uint64_t type_id) {
    uint64_t edge_id = ++catalog.connections_count;

    // TODO: special cases
    // bool node_loop = from_id == to_id;
    // if (node_loop) {
        // self_connected_nodes.append_record(RecordFactory::get(from_id, edge_id));
    // }
    from_to_type_edge.append_record( RecordFactory::get(from_id, to_id, type_id, edge_id) );
    model.edge_table->append_record(RecordFactory::get(from_id, to_id, type_id));

    return edge_id;
}


// void BulkImport::set_property_stats(map<uint64_t, pair<uint64_t, uint64_t>>& m, OrderedFile<3>& ordered_properties) {
//     uint64_t current_key = 0;
//     uint64_t current_value = 0;
//     uint64_t key_count = 0;
//     uint64_t distinct_values = 0;

//     ordered_properties.begin();
//     auto record = ordered_properties.next_record();
//     while (record != nullptr) {
//         // check same key
//         if (record->ids[0] == current_key) {
//             ++key_count;
//             // check if value changed
//             if (record->ids[1] != current_value) {
//                 ++distinct_values;
//             }
//         } else {
//             // save stats from last key
//             if (current_key != 0) {
//                 m.insert({ current_key, make_pair(key_count, distinct_values) });
//             }
//             current_key = record->ids[0];
//             current_value = record->ids[1];

//             key_count = 1;
//             distinct_values = 1;
//         }
//         record = ordered_properties.next_record();
//     }
//     // save stats from last key
//     if (current_key != 0) {
//         m.insert({ current_key, make_pair(key_count, distinct_values) });
//     }
// }
