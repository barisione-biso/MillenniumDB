#include "bulk_import.h"

#include <chrono>
#include <iostream>
#include <map>
#include <boost/spirit/include/support_istream_iterator.hpp>

#include "base/parser/grammar/common/value_visitor.h"
#include "base/parser/grammar/import/import_ast_adapted.h"
#include "base/parser/grammar/import/import_ast.h"
#include "base/parser/grammar/import/import_def.h"
#include "base/parser/grammar/import/import.h"
#include "storage/buffer_manager.h"
#include "storage/file_manager.h"

using namespace std;

BulkImport::BulkImport(const string& filename, QuadModel& model) :
    model                (model),
    catalog              (model.catalog()),
    node_labels          (OrderedFile<2>("node_labels.dat")),
    object_key_value     (OrderedFile<3>("object_key_value.dat")),
    from_to_type_edge    (OrderedFile<4>("from_to_type_edge.dat")),
    equal_from_to        (OrderedFile<3>("equal_from_to.dat")),
    equal_from_type      (OrderedFile<3>("equal_from_type.dat")),
    equal_to_type        (OrderedFile<3>("equal_to_type.dat")),
    equal_from_to_type   (OrderedFile<2>("equal_from_to_type.dat"))
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
    std::cout << "Reading files & writing ordered file...\n";

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
            cerr << "ERROR: line " << line_number << " has wrong format in import file:\n";
            auto character_count = 0;
            while (*file_iter != '\n' && file_iter != file_iter_end) {
                ++character_count;
                if (character_count > 256) {
                    cerr << "...(line exceeded 256 characters)\n";
                    return;
                } else {
                    cerr << *file_iter;
                    ++file_iter;
                }
            }
            cerr << "\n";
            return;
        }
    } while(file_iter != file_iter_end);

    auto finish_reading_files = chrono::system_clock::now();
    chrono::duration<float, milli> phase1_duration = finish_reading_files - start;
    std::cout << "  done in " << phase1_duration.count() << " ms\n\n";

    std::cout << "Creating indexes...\n";

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

    { // count total properties and distinct values
        map<uint64_t, uint64_t> map_key_count;
        map<uint64_t, uint64_t> map_distinct_values;
        uint64_t current_key     = 0;
        uint64_t current_value   = 0;
        uint64_t key_count       = 0;
        uint64_t distinct_values = 0;

        object_key_value.begin_read();
        auto record = object_key_value.next_record();
        while (record != nullptr) {
            // check same key
            if (record->ids[0] == current_key) {
                ++key_count;
                // check if value changed
                if (record->ids[1] != current_value) {
                    ++distinct_values;
                    current_value = record->ids[1];
                }
            } else {
                // save stats from last key
                if (current_key != 0) {
                    map_key_count.insert({ current_key, key_count });
                    map_distinct_values.insert({ current_key, distinct_values });
                }
                current_key   = record->ids[0];
                current_value = record->ids[1];

                key_count       = 1;
                distinct_values = 1;
            }
            record = object_key_value.next_record();
        }
        // save stats from last key
        if (current_key != 0) {
            map_key_count.insert({ current_key, key_count });
            map_distinct_values.insert({ current_key, distinct_values });
        }

        catalog.key2distinct    = move(map_distinct_values);
        catalog.key2total_count = move(map_key_count);
    }

    // CONNECTIONS
    from_to_type_edge.order(std::array<uint_fast8_t, 4> { 0, 1, 2, 3 });
    model.from_to_type_edge->bulk_import(from_to_type_edge);

    // set catalog.distinct_from
    {
        uint64_t distinct_from = 0;
        uint64_t current_from  = 0;

        from_to_type_edge.begin_read();
        auto record = from_to_type_edge.next_record();
        while (record != nullptr) {
            if (record->ids[0] != current_from) {
                ++distinct_from;
                current_from = record->ids[0];
            }
            record = from_to_type_edge.next_record();
        }
        catalog.distinct_from = distinct_from;
    }

    from_to_type_edge.order(std::array<uint_fast8_t, 4> { 2, 0, 1, 3 });
    model.to_type_from_edge->bulk_import(from_to_type_edge);

    // set catalog.distinct_to
    {
        uint64_t distinct_to = 0;
        uint64_t current_to  = 0;

        from_to_type_edge.begin_read();
        auto record = from_to_type_edge.next_record();
        while (record != nullptr) {
            if (record->ids[0] != current_to) {
                ++distinct_to;
                current_to = record->ids[0];
            }
            record = from_to_type_edge.next_record();
        }
        catalog.distinct_to = distinct_to;
    }


    from_to_type_edge.order(std::array<uint_fast8_t, 4> { 2, 0, 1, 3 });
    model.type_from_to_edge->bulk_import(from_to_type_edge);

    // SPECIAL CASES

    // Equal from to
    equal_from_to.order(std::array<uint_fast8_t, 3> { 0, 1, 2 });
    model.equal_from_to->bulk_import(equal_from_to);

    equal_from_to.order(std::array<uint_fast8_t, 3> { 1, 0, 2 });
    model.equal_from_to_inverted->bulk_import(equal_from_to);

    // calling this after inverted, so type is at pos 0
    set_distinct_type_stats(equal_from_to, catalog.type2equal_from_to_count);

    // Equal from type
    equal_from_type.order(std::array<uint_fast8_t, 3> { 0, 1, 2 });
    model.equal_from_type->bulk_import(equal_from_type);

    // calling this before inverted, so type is at pos 0
    set_distinct_type_stats(equal_from_type, catalog.type2equal_from_type_count);

    equal_from_type.order(std::array<uint_fast8_t, 3> { 1, 0, 2 });
    model.equal_from_type_inverted->bulk_import(equal_from_type);

    // Equal to type
    equal_to_type.order(std::array<uint_fast8_t, 3> { 0, 1, 2 });
    model.equal_to_type->bulk_import(equal_to_type);

    // calling this before inverted, so type is at pos 0
    set_distinct_type_stats(equal_to_type, catalog.type2equal_to_type_count);

    equal_to_type.order(std::array<uint_fast8_t, 3> { 1, 0, 2 });
    model.equal_to_type_inverted->bulk_import(equal_to_type);

    // Equal from to type
    equal_from_to_type.order(std::array<uint_fast8_t, 2> { 0, 1 });
    model.equal_from_to_type->bulk_import(equal_from_to_type);

    set_distinct_type_stats(equal_from_to_type, catalog.type2equal_from_to_type_count);

    catalog.distinct_labels = catalog.label2total_count.size();
    catalog.distinct_keys   = catalog.key2total_count.size();
    catalog.distinct_type   = catalog.type2total_count.size();

    auto finish_creating_index = chrono::system_clock::now();
    chrono::duration<float, milli> phase2_duration = finish_creating_index - finish_reading_files;
    std::cout << "  done in " << phase2_duration.count() << " ms\n\n";

    // if (catalog.identifiable_defined_count != catalog.identifiable_nodes_count) {
    //     std::cout << "WARNING: defined identifiable nodes(" << catalog.identifiable_defined_count
    //     << ") and used identifiable nodes (" << catalog.identifiable_nodes_count << ") are different.\n";
    // }
    catalog.save_changes();
}


template <std::size_t N>
void BulkImport::set_distinct_type_stats(OrderedFile<N>& ordered_file, std::map<uint64_t, uint64_t>& map) {
    map.clear();
    uint64_t current_type = 0;
    uint64_t count        = 0;

    ordered_file.begin_read();
    auto record = ordered_file.next_record();
    while (record != nullptr) {
        // check same key
        if (record->ids[0] == current_type) {
            ++count;
        } else {
            // save stats from last key
            if (current_type != 0) {
                map.insert({ current_type, count });
            }
            current_type = record->ids[0];
            count = 1;
        }
        record = ordered_file.next_record();
    }
    // save stats from last key
    if (current_type != 0) {
        map.insert({ current_type, count });
    }
}


uint64_t BulkImport::get_node_id(const string& node_name) {
    bool created;
    auto obj_id = model.get_or_create_identifiable_object_id(node_name, &created);
    if (created) {
        model.node_table->append_record(RecordFactory::get(obj_id));
        ++catalog.identifiable_nodes_count;
    } else {
        if (node_name.size() < 8) {
            auto inlined_ids_search = inlined_ids.find(obj_id);
            if (inlined_ids_search == inlined_ids.end()) {
                inlined_ids.insert(obj_id);
                model.node_table->append_record(RecordFactory::get(obj_id));
                ++catalog.identifiable_nodes_count;
            }
        }
    }
    return obj_id;
}


uint64_t BulkImport::get_anonymous_node_id(const string& tmp_name) {
    // TODO: asuming anonymous ids are sequential starting from 1
    auto str = tmp_name;
    str.erase(0, 1); // delete first character: '_'
    uint64_t unmasked_id = std::stoull(str);
    if (catalog.anonymous_nodes_count < unmasked_id) {
        catalog.anonymous_nodes_count = unmasked_id;
    }

    return unmasked_id | model.ANONYMOUS_NODE_MASK;
}


uint64_t BulkImport::process_node(const import::ast::Node node) {
    uint64_t node_id;
    if (node.anonymous()) {
        node_id = get_anonymous_node_id(node.name);
    } else {
        node_id = get_node_id(node.name);
    }

    for (auto& label : node.labels) {
        auto label_id = model.get_or_create_string_id(label);
        ++catalog.label_count;
        ++catalog.label2total_count[label_id];

        node_labels.append_record(std::array<uint64_t, 2> { node_id, label_id });
    }

    for (auto& property : node.properties) {
        ValueVisitor visitor;
        auto v = property.value;
        auto value = visitor(v);

        auto key_id   = model.get_or_create_string_id(property.key);
        auto value_id = model.get_or_create_value_id(value);

        ++catalog.properties_count;

        object_key_value.append_record(std::array<uint64_t, 3> { node_id, key_id, value_id });
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
        auto v = property.value;
        auto value = visitor(v);

        auto key_id   = model.get_or_create_string_id(property.key);
        auto value_id = model.get_or_create_value_id(value);

        ++catalog.properties_count;
        // ++catalog.key2total_count[key_id];

        object_key_value.append_record(std::array<uint64_t, 3> { edge_id, key_id, value_id });
    }
    ++catalog.type2total_count[type_id];
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
        auto v = property.value;
        auto value = visitor(v);

        auto key_id   = model.get_or_create_string_id(property.key);
        auto value_id = model.get_or_create_value_id(value);

        ++catalog.properties_count;
        // ++catalog.key2total_count[key_id];

        object_key_value.append_record(std::array<uint64_t, 3> { edge_id, key_id, value_id });
    }
    ++catalog.type2total_count[type_id];
    return edge_id;
}


uint64_t BulkImport::create_connection(const uint64_t from_id, const uint64_t to_id, const uint64_t type_id) {
    uint64_t edge_id = ++catalog.connections_count | model.CONNECTION_MASK;

    // special cases
    if (from_id == to_id) {
        ++catalog.equal_from_to_count;
        equal_from_to.append_record(std::array<uint64_t, 3> { from_id, type_id, edge_id });

        if (from_id == type_id) {
            ++catalog.equal_from_to_type_count;
            equal_from_to_type.append_record(std::array<uint64_t, 2> { from_id, edge_id });
        }
    }
    if (from_id == type_id) {
        ++catalog.equal_from_type_count;
        equal_from_type.append_record(std::array<uint64_t, 3> { from_id, to_id, edge_id });
    }
    if (to_id == type_id) {
        ++catalog.equal_to_type_count;
        equal_to_type.append_record(std::array<uint64_t, 3> { type_id, from_id, edge_id });
    }

    from_to_type_edge.append_record(std::array<uint64_t, 4> { from_id, to_id, type_id, edge_id });
    model.edge_table->append_record(std::array<uint64_t, 3> { from_id, to_id, type_id });

    return edge_id;
}
