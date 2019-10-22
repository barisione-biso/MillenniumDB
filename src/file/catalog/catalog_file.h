#ifndef FILE__CATALOG_FILE_H_
#define FILE__CATALOG_FILE_H_

#include <fstream>
#include <string>

using namespace std;

class CatalogFile
{
    public:
        CatalogFile(const string& filename);
        ~CatalogFile();

        uint64_t create_node();
        uint64_t create_edge();

        uint64_t get_node_count();
        uint64_t get_edge_count();

    private:
        fstream file;
        // cache all numbers, write to disk only on destructor
        uint64_t node_count;
        uint64_t edge_count;

        // uint64_t get(int pos);
};

#endif //FILE__CATALOG_FILE_H_
