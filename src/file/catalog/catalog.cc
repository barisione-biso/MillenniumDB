#include "catalog.h"

Catalog::Catalog(const string& filename_prefix)
{

}

Catalog::~Catalog()
{

}

u_int64_t Catalog::create_node(const string& id)
{
    // TODO:
    // obtener hash 128 bits
    // ver que no exista en bpt de NodeIds
    // obtener new_id = node_count (y aumenta node_count)
    // insertar en bpt de NodeIds nuevo record: hash|new_id
    // retornar new_id (unmasked)
    return 0;
}

u_int64_t Catalog::create_edge()
{
    // obtener new_id = edge_count (y aumenta node_count)
    // retornar new_id (unmasked)
    return 0;
}

u_int64_t Catalog::get_node_count(){}
u_int64_t Catalog::get_edge_count(){}

u_int64_t Catalog::get_node_count_for_label(u_int64_t label_id_masked){}
u_int64_t Catalog::get_edge_count_for_label(u_int64_t label_id_masked){}

u_int64_t Catalog::get_node_count_for_key(u_int64_t key_id_masked){}
u_int64_t Catalog::get_edge_count_for_key(u_int64_t key_id_masked){}