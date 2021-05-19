#include "path_manager.h"

#include <new>         // placement new
#include <type_traits> // aligned_storage

// memory for the object
static typename std::aligned_storage<sizeof(PathManager), alignof(PathManager)>::type path_manager_buf;
// global object
PathManager& path_manager = reinterpret_cast<PathManager&>(path_manager_buf);

PathManager::PathManager(QuadModel& model) :
    model (model) { }


void PathManager::init(QuadModel& model) {
    new (&path_manager) PathManager(model); // placement new
}


ObjectId PathManager::set_path(const SearchState* visited_pointer, uint_fast32_t property_path_id) {
    saved_visited_pointer = visited_pointer;
    return ObjectId(GraphModel::VALUE_PATH_MASK | 0);
}


void PathManager::print(std::ostream& os, uint64_t path_id) const {
    auto current_state = saved_visited_pointer; // TODO: set from path_id
    while (current_state != nullptr) {
       os << model.get_graph_object(current_state->object_id) << "<=";
       current_state = const_cast<SearchState*>(current_state->previous);
    }
    os << "\n";
}