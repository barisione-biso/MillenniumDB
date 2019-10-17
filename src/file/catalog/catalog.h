#ifndef FILE__CATALOG_H_
#define FILE__CATALOG_H_

#include <memory>
#include <string>

using namespace std;

class Catalog
{
    public:
        Catalog(const string& filename_prefix);
        ~Catalog(); // not defaulted, write to disk

        // create_node and create_edge return the new id without the corresponding mask
        u_int64_t create_node(const string& id);
        u_int64_t create_edge();

        u_int64_t get_node_count();
        u_int64_t get_edge_count();

        u_int64_t get_node_count_for_label(u_int64_t label_id_masked);
        u_int64_t get_edge_count_for_label(u_int64_t label_id_masked);

        u_int64_t get_node_count_for_key(u_int64_t key_id_masked);
        u_int64_t get_edge_count_for_key(u_int64_t key_id_masked);

    private:
        unique_ptr<Catalog> file;
};

#endif //FILE__CATALOG_H_
