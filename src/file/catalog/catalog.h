#ifndef FILE__CATALOG_H_
#define FILE__CATALOG_H_

#include <memory>
#include <map>
#include <string>

using namespace std;

class Catalog
{
    public:
        Catalog(const string& filename_prefix);
        ~Catalog(); // not defaulted, write to disk

        // create_node and create_edge return the new id without the corresponding mask
        uint64_t create_node();
        uint64_t create_edge();

        uint64_t get_node_count();
        uint64_t get_edge_count();

        uint64_t get_node_count_for_label(uint64_t label_id_masked);
        uint64_t get_edge_count_for_label(uint64_t label_id_masked);

        uint64_t get_node_count_for_key(uint64_t key_id_masked);
        uint64_t get_edge_count_for_key(uint64_t key_id_masked);

    private:
        unique_ptr<Catalog> file;
        uint64_t node_count;
        uint64_t edge_count;
        uint64_t node_label_count;
        uint64_t edge_label_count;
        uint64_t node_key_count;
        uint64_t edge_key_count;

        map<uint64_t, uint64_t> node_label_stats;
        map<uint64_t, uint64_t> edge_label_stats;
        map<uint64_t, uint64_t> node_key_stats;
        map<uint64_t, uint64_t> label_stats;
};

#endif //FILE__CATALOG_H_
