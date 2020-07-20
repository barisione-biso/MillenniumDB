/**
 * Catalog saves:
 * - number of graphs
 * - graph names and ids
 *
 **/

#ifndef STORAGE__CATALOG_H_
#define STORAGE__CATALOG_H_

#include <fstream>
#include <memory>
#include <map>
#include <string>
#include <vector>

#include "base/ids/graph_id.h"
#include "base/ids/object_id.h"

class Catalog {
public:
    static constexpr auto catalog_file_name = "catalog.dat";
    ~Catalog();

    static void init();

    // Create_node and create_edge return the new id without the corresponding mask
    // boths methods must skip id = 0 (reserved for null ids)
    uint64_t create_node(GraphId);
    uint64_t create_edge(GraphId, bool node_loop);

    uint64_t get_node_count(GraphId);
    uint64_t get_edge_count(GraphId);
    uint64_t get_node_loop_count(GraphId);

    uint64_t get_node_labels(GraphId);
    uint64_t get_edge_labels(GraphId);
    uint64_t get_node_distinct_labels(GraphId);
    uint64_t get_edge_distinct_labels(GraphId);

    uint64_t get_node_properties(GraphId);
    uint64_t get_edge_properties(GraphId);
    uint64_t get_node_distinct_properties(GraphId);
    uint64_t get_edge_distinct_properties(GraphId);

    // IDs received are unmasked (no type or graph)
    void add_node_label(GraphId, uint64_t label_id);
    void add_edge_label(GraphId, uint64_t label_id);

    void set_node_properties_stats(GraphId, uint64_t property_count,
                                   std::map<uint64_t, std::pair<uint64_t, uint64_t>>);
    void set_edge_properties_stats(GraphId, uint64_t property_count,
                                   std::map<uint64_t, std::pair<uint64_t, uint64_t>>);

    uint64_t get_node_label_count(GraphId, ObjectId label_id);
    uint64_t get_edge_label_count(GraphId, ObjectId label_id);

    uint64_t get_node_key_count(GraphId, ObjectId key_id);
    uint64_t get_edge_key_count(GraphId, ObjectId key_id);

    uint64_t get_node_key_unique_values(GraphId,  ObjectId key_id);
    uint64_t get_edge_key_unique_values(GraphId, ObjectId key_id);

    // does not count the default graph
    uint_fast16_t get_graph_count();

    GraphId create_graph(const std::string& graph_name);
    GraphId get_graph(const std::string& graph_name);

    void print();

private:
    Catalog();

    std::fstream* file;
    uint_fast16_t graph_count;

    std::vector<std::string> graph_names;
    std::map<std::string, GraphId> graph_ids;

    std::vector<uint64_t> node_count;
    std::vector<uint64_t> node_label_count;
    std::vector<uint64_t> node_property_count;

    std::vector<uint64_t> edge_count;
    std::vector<uint64_t> edge_label_count;
    std::vector<uint64_t> edge_property_count;

    std::vector<uint64_t> node_loop_count;

    std::vector<std::map<uint64_t, uint64_t>> node_label_stats;
    std::vector<std::map<uint64_t, uint64_t>> edge_label_stats;

    // key_id -> (count, distinct_values)
    std::vector<std::map<uint64_t, std::pair<uint64_t, uint64_t>>> node_key_stats;
    std::vector<std::map<uint64_t, std::pair<uint64_t, uint64_t>>> edge_key_stats;

    uint64_t read_uint64();
    uint_fast32_t read_uint32();

    void write_uint64(uint64_t);
    void write_uint32(uint_fast32_t);

    void flush();
    // add (key, 1) to a map if key is not present or increase value by 1 otherwise
    void add_to_map(std::map<uint64_t, uint64_t>& map, uint64_t key);
    uint64_t get_map_value(std::map<uint64_t, uint64_t>& map, uint64_t key);
};

extern Catalog& catalog; // global object

#endif // STORAGE__CATALOG_H_
