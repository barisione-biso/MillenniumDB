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
    }
    else {
        file.seekg(0, file.beg);
        char buffer[8];

        file.read(buffer, 8);
        node_count = *(u_int64_t *)buffer;

        file.read(buffer, 8);
        edge_count = *(u_int64_t *)buffer;
    }
}

CatalogFile::~CatalogFile()
{
    file.seekg(0, file.beg);
    file.write((const char *)&node_count, 8);
    file.write((const char *)&edge_count, 8);
    file.close();
}

u_int64_t CatalogFile::create_node()
{
    return ++node_count;
}

u_int64_t CatalogFile::create_edge()
{
    return ++edge_count;
}

u_int64_t CatalogFile::get_node_count()
{
    return node_count;
}

u_int64_t CatalogFile::get_edge_count()
{
    return edge_count;
}

// u_int64_t CatalogFile::get(int pos)
// {
//     file.seekg(pos);
//     char bytes[8];
//     file.read(bytes, 8);
//     return *(u_int64_t *)bytes;
// }
