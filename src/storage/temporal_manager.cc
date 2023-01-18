#include "temporal_manager.h"

#include <cassert>
#include <sys/mman.h>

#include "base/exceptions.h"
#include "execution/graph_object/char_iter.h"
// #include "storage/file_manager.h"

// memory for the object
static typename std::aligned_storage<sizeof(TemporalManager), alignof(TemporalManager)>::type temporal_manager_buf;
// global object
TemporalManager& temporal_manager = reinterpret_cast<TemporalManager&>(temporal_manager_buf);


void TemporalManager::init() {
    new (&temporal_manager) TemporalManager(); // placement new
}


TemporalManager::TemporalManager(){
    next_str_id = 0;
    next_dtt_id = 0;
    next_lan_id = 0;
}


TemporalManager::~TemporalManager() {}


void TemporalManager::print_str(std::ostream& os, uint64_t id) const {
    if (id >= next_str_id) return; // handle error?
    std::string str = id_to_str.find(id)->second;
    os << str;
}

void TemporalManager::print_dtt(std::ostream& os, uint64_t id) const {
    if (id >= next_dtt_id) return; // handle error?
    std::string str = id_to_dtt.find(id)->second;
    os << str;
}

void TemporalManager::print_lan(std::ostream& os, uint64_t id) const {
    if (id >= next_lan_id) return; // handle error?
    std::string str = id_to_lan.find(id)->second;
    os << str;
}

uint64_t TemporalManager::get_str_id(const std::string& str) {
    auto id = str_to_id.find(str);
    if (id != str_to_id.end()) return id->second;
    str_to_id[str] = next_str_id;
    id_to_str[next_str_id] = str;
    ++next_str_id;
    return next_str_id - 1;
}

uint64_t TemporalManager::get_dtt_id(const std::string& dtt) {
    auto id = dtt_to_id.find(dtt);
    if (id != dtt_to_id.end()) return id->second;
    dtt_to_id[dtt] = next_dtt_id;
    id_to_dtt[next_dtt_id] = dtt;
    ++next_dtt_id;
    return next_dtt_id - 1;
}

uint64_t TemporalManager::get_lan_id(const std::string& lan) {
    auto id = lan_to_id.find(lan);
    if (id != lan_to_id.end()) return id->second;
    lan_to_id[lan] = next_lan_id;
    id_to_lan[next_lan_id] = lan;
    ++next_lan_id;
    return next_lan_id - 1;
}