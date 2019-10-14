#include "catalog_file.h"

CatalogFile::CatalogFile(const string& filename)
{
    file = fstream(filename, ios::in|ios::out|ios::binary);
    file.seekg (0, file.end);
    int last_pos = file.tellg();
    if (last_pos == -1) {
        file.close();
        std::ofstream tmp(filename);
        tmp.close();
        file = fstream(filename, ios::in|ios::out|ios::binary);

        // Initialize with zeros
        node_count = 0;
        edge_count = 0;
        current_max_node = 0; // First node will be node(1), not node(0)
        current_max_edge = 0; // First edge will be edge(1), not edge(0)

    }
    else {
        file.seekg(0, file.beg);
        char buffer[8];

        file.read(buffer, 8);
        node_count = *(u_int64_t *)buffer;

        file.read(buffer, 8);
        edge_count = *(u_int64_t *)buffer;

        file.read(buffer, 8);
        current_max_node = *(u_int64_t *)buffer;

        file.read(buffer, 8);
        current_max_edge = *(u_int64_t *)buffer;
    }
}

CatalogFile::~CatalogFile()
{
    file.seekg(0, file.beg);
    file.write((const char *)&node_count, 8);
    file.write((const char *)&edge_count, 8);
    file.write((const char *)&current_max_node, 8);
    file.write((const char *)&current_max_edge, 8);
    file.close();
}

u_int64_t CatalogFile::create_node()
{
    ++node_count;
    return ++current_max_node;
}

u_int64_t CatalogFile::create_edge()
{
    ++edge_count;
    return ++current_max_edge;
}

u_int64_t CatalogFile::get_node_count()
{
    return node_count;
}

u_int64_t CatalogFile::get_edge_count()
{
    return edge_count;
}

void CatalogFile::create_node(u_int64_t id)
{
    ++node_count;
    if (id > current_max_node) {
        current_max_node = id;
    }
}

void CatalogFile::create_edge(u_int64_t id)
{
    ++edge_count;
    if (id > current_max_edge) {
        current_max_edge = id;
    }
}

u_int64_t CatalogFile::get(int pos)
{
    file.seekg(pos);
    char bytes[8];
    file.read(bytes, 8);
    return *(u_int64_t *)bytes;
}
