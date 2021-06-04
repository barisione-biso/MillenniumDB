#ifndef RELATIONAL_MODEL__PATH_MANAGER_H_
#define RELATIONAL_MODEL__PATH_MANAGER_H_

#include <vector>
#include "base/ids/object_id.h"
#include "relational_model/models/quad_model/quad_model.h"
#include "relational_model/execution/binding_id_iter/property_paths/search_state.h"


// PathManager converts a path into a object id


class PathManager : public PathPrinter {
public:
    ~PathManager() = default;

    ObjectId set_path(const SearchState* visited_pointer, VarId path_var);
    void print(std::ostream& os, uint64_t path_id) const override;

    static void init(QuadModel& model);

private:
    QuadModel& model; // TODO: ojala fuese GraphModel
    PathManager(QuadModel& model);
    std::vector<const SearchState*> paths;
};

extern PathManager& path_manager; // global object

#endif // RELATIONAL_MODEL__PATH_MANAGER_H_
