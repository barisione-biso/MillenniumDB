#ifndef FILE__CATALOG_FILE_
#define FILE__CATALOG_FILE_

#include <fstream>
#include <string>

using namespace std;

class CatalogFile
{
    public:
        CatalogFile(const string& filename);
        ~CatalogFile();

        u_int64_t create_node();
        u_int64_t create_edge();

        void create_node(u_int64_t id);
        void create_edge(u_int64_t id);

        u_int64_t get_node_count();
        u_int64_t get_edge_count();

    private:
        fstream file;
        // cache all numbers, write to disk only on destructor
        u_int64_t current_max_node;
        u_int64_t current_max_edge;
        u_int64_t node_count;
        u_int64_t edge_count;

        u_int64_t get(int pos);
};

#endif //FILE__CATALOG_FILE_
