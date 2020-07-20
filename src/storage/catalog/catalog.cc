#include "catalog.h"

#include <cassert>
#include <iostream>
#include <new>         // placement new
#include <type_traits> // aligned_storage

#include "base/parser/logical_plan/exceptions.h"
#include "storage/file_manager.h"

using namespace std;

// memory for the object
static typename aligned_storage<sizeof(Catalog), alignof(Catalog)>::type catalog_buf;
// global object
Catalog& catalog = reinterpret_cast<Catalog&>(catalog_buf);

Catalog::Catalog() {
    file = &file_manager.get_file(file_manager.get_file_id(catalog_file_name));

    file->seekg(0, file->end);
    if (file->tellg() == 0) {
        graph_count = 0;
        node_count           = vector<uint64_t>(1);
        node_label_count     = vector<uint64_t>(1);
        node_property_count  = vector<uint64_t>(1);

        edge_count           = vector<uint64_t>(1);
        edge_label_count     = vector<uint64_t>(1);
        edge_property_count  = vector<uint64_t>(1);

        node_label_stats = vector<map<uint64_t, uint64_t>>(1);
        edge_label_stats = vector<map<uint64_t, uint64_t>>(1);
        node_key_stats   = vector<map<uint64_t, pair<uint64_t, uint64_t>>>(1);
        edge_key_stats   = vector<map<uint64_t, pair<uint64_t, uint64_t>>>(1);

        node_count[0]           = 0;
        node_label_count[0]     = 0;
        node_property_count[0]  = 0;

        edge_count[0]           = 0;
        edge_label_count[0]     = 0;
        edge_property_count[0]  = 0;

        graph_names.push_back("Default Graph");
    }
    else {
        file->seekg(0, file->beg);

        graph_count = read_uint32();
        if (graph_count >= 65536 /*2^16*/ ) {
            throw logic_error("Catalog file inconsistent: graph_count must be less than 2^16 (65536).");
            return;
        }
        if (graph_count == 0) {
            throw logic_error("Catalog file inconsistent: graph_count must be more than 0.");
            return;
        }
        node_count           = vector<uint64_t>(graph_count + 1);
        node_label_count     = vector<uint64_t>(graph_count + 1);
        node_property_count  = vector<uint64_t>(graph_count + 1);

        edge_count           = vector<uint64_t>(graph_count + 1);
        edge_label_count     = vector<uint64_t>(graph_count + 1);
        edge_property_count  = vector<uint64_t>(graph_count + 1);

        node_label_stats = vector<map<uint64_t, uint64_t>>(graph_count + 1);
        edge_label_stats = vector<map<uint64_t, uint64_t>>(graph_count + 1);
        node_key_stats   = vector<map<uint64_t, pair<uint64_t, uint64_t>>>(graph_count + 1);
        edge_key_stats   = vector<map<uint64_t, pair<uint64_t, uint64_t>>>(graph_count + 1);


        for (uint32_t graph = 0; graph <= graph_count; graph++) {
            auto graph_name_length  = read_uint32();
            string graph_name = string(graph_name_length, ' ');
            file->read((char*)graph_name.data(), graph_name_length);

            graph_names.push_back(graph_name);
            graph_ids.insert({graph_name, GraphId(graph)});

            node_count[graph]           = read_uint64();
            node_label_count[graph]     = read_uint64();
            node_property_count[graph]  = read_uint64();
            auto node_distinct_labels   = read_uint64();
            auto node_distinct_keys     = read_uint64();

            edge_count[graph]           = read_uint64();
            edge_label_count[graph]     = read_uint64();
            edge_property_count[graph]  = read_uint64();
            auto edge_distinct_labels   = read_uint64();
            auto edge_distinct_keys     = read_uint64();

            for (uint64_t i = 0; i < node_distinct_labels; i++) {
                auto label_id = read_uint64();
                auto label_count = read_uint64();
                node_label_stats[graph].insert({label_id, label_count});
            }
            for (uint64_t i = 0; i < node_distinct_keys; i++) {
                auto key_id = read_uint64();
                auto key_count = read_uint64();
                auto distinct_values = read_uint64();
                node_key_stats[graph].insert({key_id, make_pair(key_count, distinct_values)});
            }
            for (uint64_t i = 0; i < edge_distinct_labels; i++) {
                auto label_id = read_uint64();
                auto label_count = read_uint64();
                edge_label_stats[graph].insert({label_id, label_count});
            }
            for (uint64_t i = 0; i < edge_distinct_keys; i++) {
                auto key_id = read_uint64();
                auto key_count = read_uint64();
                auto distinct_values = read_uint64();
                edge_key_stats[graph].insert({key_id, make_pair(key_count, distinct_values)});
            }
        }
    }
}

void Catalog::init() {
    new (&catalog) Catalog(); // placement new
}


Catalog::~Catalog() {
    flush();
}


GraphId Catalog::get_graph(const string& graph_name) {
    if (graph_name.empty()) {
        return GraphId(GraphId::DEFAULT_GRAPH_ID);
    }
    auto search = graph_ids.find(graph_name);
    if (search == graph_ids.end()) {
        throw GraphDoesNotExist(graph_name);
    }
    return search->second;
}


GraphId Catalog::create_graph(const string& graph_name) {
    if (graph_name.empty()) {
        throw invalid_argument("Graph must have a name.");
    }
    else {
        auto search = graph_ids.find(graph_name);
        if (search != graph_ids.end()) {
            throw invalid_argument("\"" + graph_name + "\" graph name already exixsts.");
        }
    }
    auto graph_id = GraphId(++graph_count);
    graph_ids.insert({graph_name, graph_id});
    graph_names.push_back(graph_name);

    node_count.push_back(0);
    node_label_count.push_back(0);
    node_property_count.push_back(0);

    edge_count.push_back(0);
    edge_label_count.push_back(0);
    edge_property_count.push_back(0);

    node_label_stats.push_back(map<uint64_t, uint64_t>());
    node_key_stats.push_back(map<uint64_t, pair<uint64_t, uint64_t>>());

    edge_label_stats.push_back(map<uint64_t, uint64_t>());
    edge_key_stats.push_back(map<uint64_t, pair<uint64_t, uint64_t>>());

    return graph_id;
}


void Catalog::flush() {
    // print();
    file->seekg(0, file->beg);
    write_uint32(graph_count);

    for (uint_fast16_t graph = 0; graph <= graph_count; graph++) {
        uint_fast32_t name_len = graph_names[graph].size();

        write_uint32(name_len);
        file->write((const char *)graph_names[graph].data(), name_len);

        write_uint64(node_count[graph]);
        write_uint64(node_label_count[graph]);
        write_uint64(node_property_count[graph]);
        write_uint64(node_label_stats[graph].size());
        write_uint64(node_key_stats[graph].size());

        write_uint64(edge_count[graph]);
        write_uint64(edge_label_count[graph]);
        write_uint64(edge_property_count[graph]);
        write_uint64(edge_label_stats[graph].size());
        write_uint64(edge_key_stats[graph].size());

        for (auto&& [id, count] : node_label_stats[graph]) {
            write_uint64(id);
            write_uint64(count);
        }
        for (auto&& [id, s] : node_key_stats[graph]) {
            write_uint64(id);
            write_uint64(s.first);
            write_uint64(s.second);
        }
        for (auto&& [id, count] : edge_label_stats[graph]) {
            write_uint64(id);
            write_uint64(count);
        }
        for (auto&& [id, s] : edge_key_stats[graph]) {
            write_uint64(id);
            write_uint64(s.first);
            write_uint64(s.second);
        }
    }
}


void Catalog::print() {
    for (uint_fast16_t graph = 0; graph <= graph_count; graph++) {
        cout << graph_names[graph] << ":" << endl;
        cout << "  node count:          " << node_count[graph] << endl;
        cout << "  node labels:         " << node_label_count[graph] << endl;
        cout << "  node properties:     " << node_property_count[graph] << endl;
        cout << "  node disinct labels: " << node_label_stats[graph].size() << endl;
        cout << "  node disinct keys:   " << node_key_stats[graph].size() << endl;

        cout << "  edge count:          " << edge_count[graph] << endl;
        cout << "  edge labels:         " << edge_label_count[graph] << endl;
        cout << "  edge properties:     " << edge_property_count[graph] << endl;
        cout << "  edge disinct labels: " << edge_label_stats[graph].size() << endl;
        cout << "  edge disinct keys:   " << edge_key_stats[graph].size() << endl;

    }
}


uint64_t Catalog::read_uint64() {
    uint64_t res = 0;
    uint8_t buf[8];
    file->read((char*)buf, sizeof(buf));

    for (int i = 0, shift = 0; i < 8; ++i, shift += 8) {
        res |= static_cast<uint64_t>(buf[i]) << shift;
    }
    return res;
}


uint_fast32_t Catalog::read_uint32() {
    uint_fast32_t res = 0;
    uint8_t buf[4];
    file->read((char*)buf, sizeof(buf));

    for (int i = 0, shift = 0; i < 4; ++i, shift += 8) {
        res |= static_cast<uint_fast32_t>(buf[i]) << shift;
    }
    return res;
}


void Catalog::write_uint64(uint64_t n) {
    uint8_t buf[8];
    for (unsigned int i = 0, shift = 0; i < sizeof(buf); ++i, shift += 8) {
        buf[i] = (n >> shift) & 0xFF;
    }
    file->write(reinterpret_cast<const char*>(buf), sizeof(buf));
}


void Catalog::write_uint32(uint_fast32_t n) {
    uint8_t buf[4];
    for (unsigned int i = 0, shift = 0; i < sizeof(buf); ++i, shift += 8) {
        buf[i] = (n >> shift) & 0xFF;
    }
    file->write(reinterpret_cast<const char*>(buf), sizeof(buf));
}


uint64_t Catalog::get_node_count(GraphId graph_id) {
    return node_count[graph_id];
}


uint64_t Catalog::get_edge_count(GraphId graph_id) {
    return edge_count[graph_id];
}


uint64_t Catalog::get_node_loop_count(GraphId graph_id) {
    return node_loop_count[graph_id];
}


uint64_t Catalog::get_node_labels(GraphId graph_id) {
    return node_label_count[graph_id];
}


uint64_t Catalog::get_edge_labels(GraphId graph_id) {
    return edge_label_count[graph_id];
}


uint64_t Catalog::get_node_distinct_labels(GraphId graph_id) {
    return node_label_stats[graph_id].size();
}


uint64_t Catalog::get_edge_distinct_labels(GraphId graph_id) {
    return edge_label_stats[graph_id].size();
}


uint64_t Catalog::get_node_properties(GraphId graph_id) {
    return node_property_count[graph_id];
}


uint64_t Catalog::get_edge_properties(GraphId graph_id) {
    return edge_property_count[graph_id];
}


uint64_t Catalog::get_node_distinct_properties(GraphId graph_id) {
    return node_key_stats[graph_id].size();
}


uint64_t Catalog::get_edge_distinct_properties(GraphId graph_id) {
    return edge_key_stats[graph_id].size();
}

uint64_t Catalog::create_node(GraphId graph_id) {
    assert(graph_id != 0 && "shouldn't call create_node for default graph");
    ++node_count[0];
    return ++node_count[graph_id];
}


uint64_t Catalog::create_edge(GraphId graph_id, bool node_loop) {
    assert(graph_id != 0 && "shouldn't call create_edge for default graph");
    ++edge_count[0];

    if (node_loop) {
        ++node_loop_count[0];
        ++node_loop_count[graph_id];
    }
    return ++edge_count[graph_id];
}


void Catalog::add_node_label(GraphId graph_id, uint64_t label_id) {
    assert(graph_id != 0 && "shouldn't call add_node_label for default graph");
    ++node_label_count[0];
    ++node_label_count[graph_id];
    add_to_map(node_label_stats[0], label_id);
    add_to_map(node_label_stats[graph_id], label_id);
}


void Catalog::add_edge_label(GraphId graph_id, uint64_t label_id) {
    assert(graph_id != 0 && "shouldn't call add_edge_label for default graph");
    ++edge_label_count[0];
    ++edge_label_count[graph_id];
    add_to_map(edge_label_stats[0], label_id);
    add_to_map(edge_label_stats[graph_id], label_id);
}


void Catalog::set_node_properties_stats(GraphId graph_id, uint64_t property_count,
                                        map<uint64_t, pair<uint64_t, uint64_t>> m)
{
    assert(graph_id != 0 && "shouldn't call set_node_properties_stats for default graph");
    node_property_count[0] += property_count;
    node_property_count[graph_id] = property_count;
    node_key_stats[graph_id] = move(m);
    // TODO: default graph stats
    node_key_stats[0] = node_key_stats[graph_id];
}


void Catalog::set_edge_properties_stats(GraphId graph_id, uint64_t property_count,
                                        map<uint64_t, pair<uint64_t, uint64_t>> m)
{
    assert(graph_id != 0 && "shouldn't call set_edge_properties_stats for default graph");
    edge_property_count[0] += property_count;
    edge_property_count[graph_id] = property_count;
    edge_key_stats[graph_id] = move(m);
    // TODO: default graph stats
    edge_key_stats[0] = edge_key_stats[graph_id];

}


void Catalog::add_to_map(map<uint64_t, uint64_t>& map, uint64_t key) {
    auto it = map.find(key);
    if ( it == map.end() ) {
        map.insert(pair<uint64_t, uint64_t>(key, 1));
    } else {
        it->second += 1;
    }
}


uint64_t Catalog::get_map_value(map<uint64_t, uint64_t>& map, uint64_t key) {
    auto it = map.find(key);
    if ( it == map.end() ) {
        return 0;
    } else {
        return it->second;
    }
}


uint_fast16_t Catalog::get_graph_count() {
    return graph_count;
}


uint64_t Catalog::get_node_label_count(GraphId graph_id, ObjectId label_id) {
    return get_map_value(node_label_stats[graph_id], label_id.id);
}


uint64_t Catalog::get_edge_label_count(GraphId graph_id, ObjectId label_id) {
    return get_map_value(edge_label_stats[graph_id], label_id.id);
}


uint64_t Catalog::get_node_key_count(GraphId graph_id, ObjectId key_id) {
    auto& map = node_key_stats[graph_id];
    auto it = map.find(key_id.id);
    if ( it == map.end() ) {
        return 0;
    } else {
        return it->second.first;
    }
}


uint64_t Catalog::get_edge_key_count(GraphId graph_id, ObjectId key_id){
    auto& map = edge_key_stats[graph_id];
    auto it = map.find(key_id.id);
    if ( it == map.end() ) {
        return 0;
    } else {
        return it->second.first;
    }
}

uint64_t Catalog::get_node_key_unique_values(GraphId graph_id, ObjectId key_id) {
    auto& map = node_key_stats[graph_id];
    auto it = map.find(key_id.id);
    if ( it == map.end() ) {
        return 0;
    } else {
        return it->second.second;
    }
}


uint64_t Catalog::get_edge_key_unique_values(GraphId graph_id, ObjectId key_id) {
    auto& map = edge_key_stats[graph_id];
    auto it = map.find(key_id.id);
    if ( it == map.end() ) {
        return 0;
    } else {
        return it->second.second;
    }
}
