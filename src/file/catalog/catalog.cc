#include "catalog.h"

Catalog::Catalog(const string& filename_prefix)
{

}

Catalog::~Catalog()
{

}

uint64_t Catalog::create_node()
{
    // obtener hash 128 bits
    // ver que no exista en bpt de NodeIds
    // obtener new_id = node_count (y aumenta node_count)
    // insertar en bpt de NodeIds nuevo record: hash|new_id
    // retornar new_id (unmasked)
    return node_count++;
}

uint64_t Catalog::create_edge()
{
    // obtener new_id = edge_count (y aumenta node_count)
    // retornar new_id (unmasked)
    return edge_count++;
}

uint64_t Catalog::get_node_count(){ return node_count; }
uint64_t Catalog::get_edge_count(){ return edge_count; }

uint64_t Catalog::get_node_count_for_label(uint64_t label_id_masked){ return 0; }
uint64_t Catalog::get_edge_count_for_label(uint64_t label_id_masked){ return 0; }

uint64_t Catalog::get_node_count_for_key(uint64_t key_id_masked){ return 0; }
uint64_t Catalog::get_edge_count_for_key(uint64_t key_id_masked){ return 0; }