#include "catalog.h"
#include "file/file_manager.h"


Catalog::Catalog(const string& filename)
    : file (FileManager::get_file(FileManager::get_file_id(filename)))
{
    // file = fstream(filename, ios::in|ios::out|ios::binary);
    file.seekg (0, file.end);
    if (file.tellg() == 0) {
        file.close();
        std::ofstream tmp(filename);
        tmp.close();
        file = fstream(filename, ios::in|ios::out|ios::binary);

        // Initialize with zeros
        node_count = 0;
        edge_count = 0;
        node_label_count = 0;
        edge_label_count = 0;
        node_key_count = 0;
        edge_key_count = 0;
    }
    else {
        file.seekg(0, file.beg);

        node_count = read();
        edge_count = read();
        node_label_count = read();
        edge_label_count = read();
        node_key_count = read();
        edge_key_count = read();

        for (uint32_t i = 0; i < node_label_count; i++) {
            node_label_stats.insert(read_pair());
        }
        for (uint32_t i = 0; i < edge_label_count; i++) {
            edge_label_stats.insert(read_pair());
        }
        for (uint32_t i = 0; i < node_key_count; i++) {
            node_key_stats.insert(read_pair());
        }
        for (uint32_t i = 0; i < edge_key_count; i++) {
            edge_key_stats.insert(read_pair());
        }
    }
}

Catalog::~Catalog() {
    save_changes();
    file.close();
}

void Catalog::save_changes(){
    file.seekg(0, file.beg);
    file.write((const char *)&node_count, 8);
    file.write((const char *)&edge_count, 8);
    file.write((const char *)&node_label_count, 8);
    file.write((const char *)&edge_label_count, 8);
    file.write((const char *)&node_key_count, 8);
    file.write((const char *)&edge_key_count, 8);

    for (auto&& [id, count] : node_label_stats) {
        file.write((const char *)&id, 8);
        file.write((const char *)&count, 8);
    }
    for (auto&& [id, count] : edge_label_stats) {
        file.write((const char *)&id, 8);
        file.write((const char *)&count, 8);
    }
    for (auto&& [id, count] : node_key_stats) {
        file.write((const char *)&id, 8);
        file.write((const char *)&count, 8);
    }
    for (auto&& [id, count] : edge_key_stats) {
        file.write((const char *)&id, 8);
        file.write((const char *)&count, 8);
    }
}

uint64_t Catalog::read() {
    char buffer[8];
    file.read(buffer, 8);
    return *(uint64_t *)buffer;
}

pair<uint64_t, uint64_t> Catalog::read_pair() {
    char buffer[8];
    file.read(buffer, 8);
    uint64_t first = *(uint64_t *)buffer;
    file.read(buffer, 8);
    uint64_t second = *(uint64_t *)buffer;
    return pair<uint64_t, uint64_t>(first, second);
}

uint64_t Catalog::create_node() {
    return ++node_count;
}

uint64_t Catalog::create_edge() {
    return ++edge_count;
}

uint64_t Catalog::get_node_count(){ return node_count; }
uint64_t Catalog::get_edge_count(){ return edge_count; }

void Catalog::add_node_label(uint64_t label_id) {
    add_to_map(node_label_stats, label_id);
}

void Catalog::add_edge_label(uint64_t label_id) {
    add_to_map(edge_label_stats, label_id);
}

void Catalog::add_node_key(uint64_t key_id) {
    add_to_map(node_key_stats, key_id);
}

void Catalog::add_edge_key(uint64_t key_id) {
    add_to_map(edge_key_stats, key_id);
}

void Catalog::add_to_map(map<uint64_t, uint64_t>& map, uint64_t key) {
    auto it = map.find(key);
    if ( it == map.end() ) {
        map.insert(pair<uint64_t, uint64_t>(key, 1));
    } else {
        it->second += 1;
    }
}

uint64_t Catalog::get_count(map<uint64_t, uint64_t>& map, uint64_t key) {
    auto it = map.find(key);
    if ( it == map.end() ) {
        return 0;
    } else {
        return it->second;
    }
}

uint64_t Catalog::get_node_count_for_label(uint64_t label_id) {
    return get_count(node_label_stats, label_id);
}

uint64_t Catalog::get_edge_count_for_label(uint64_t label_id) {
    return get_count(edge_label_stats, label_id);
}

uint64_t Catalog::get_node_count_for_key(uint64_t key_id) {
    return get_count(node_key_stats, key_id);
}

uint64_t Catalog::get_edge_count_for_key(uint64_t key_id){
    return get_count(edge_key_stats, key_id);
}