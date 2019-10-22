#ifndef FILE__CATALOG_H_
#define FILE__CATALOG_H_

#include <fstream>
#include <memory>
#include <map>
#include <string>

using namespace std;

class Catalog
{
    public:
        Catalog(const string& filename);
        ~Catalog(); // not defaulted, write to disk

        // create_node and create_edge return the new id without the corresponding mask
        uint64_t create_node();
        uint64_t create_edge();

        uint64_t get_node_count();
        uint64_t get_edge_count();

        // ALL IDS ARE UNMASKED
        void add_node_label(uint64_t label_id);
        void add_edge_label(uint64_t label_id);
        void add_node_key(uint64_t key_id);
        void add_edge_key(uint64_t key_id);

        uint64_t get_node_count_for_label(uint64_t label_id);
        uint64_t get_edge_count_for_label(uint64_t label_id);

        uint64_t get_node_count_for_key(uint64_t key_id);
        uint64_t get_edge_count_for_key(uint64_t key_id);

    private:
        fstream file;

        uint64_t node_count;
        uint64_t edge_count;
        uint64_t node_label_count;
        uint64_t edge_label_count;
        uint64_t node_key_count;
        uint64_t edge_key_count;

        map<uint64_t, uint64_t> node_label_stats;
        map<uint64_t, uint64_t> edge_label_stats;
        map<uint64_t, uint64_t> node_key_stats;
        map<uint64_t, uint64_t> edge_key_stats;

        void save_changes();
        void add_to_map(map<uint64_t, uint64_t>& map, uint64_t key);
        uint64_t get_count(map<uint64_t, uint64_t>& map, uint64_t key);
        uint64_t read();
        pair<uint64_t, uint64_t> read_pair();
};

#endif //FILE__CATALOG_H_
