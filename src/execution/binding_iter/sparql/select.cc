#include "select.h"

#include "execution/binding_id_iter/paths/path_manager.h"

using namespace std;
using namespace SPARQL;

Select::Select(std::unique_ptr<BindingIter>         child_iter,
               std::vector<std::pair<Var, VarId>>&& projection_vars,
               uint64_t                             limit,
               uint64_t                             offset) :
    child_iter         (move(child_iter)),
    projection_vars    (move(projection_vars)),
    limit              (limit),
    offset             (offset) { }

Select::~Select() {
    // TODO: We always have the Select operator as the root of our physical query plans.
    // If that changes we might need to call path_manager.clear() somewhere else
    // (it needs to be called always at the destruction of the query and only once)
    path_manager.clear();
}

void Select::begin(std::ostream& _os) {
    os = &_os;
    child_iter->begin(_os);

    auto it = projection_vars.cbegin();
    // Empty projection
    if (it == projection_vars.cend()) {
        (*os) << "{\"head\":{\"vars\":[]},\"results\":{\"bindings\":[";
    }
    else {
        // print header
        (*os) << "{\"head\":{\"vars\":[";
        (*os) << '"' << it->first << '"';
        while (++it != projection_vars.cend()) {
            (*os) << ",\"" << it->first << '"';
        }
        (*os) << "]},\"results\":{\"bindings\": [";
    }
}

bool Select::next() {
    while (offset > 0) {
        if (child_iter->next()) {
            --offset;
        } else {
            return false;
        }
    }

    // json format
    if (count < limit && child_iter->next()) {
        if (count > 0) {
            (*os) << ',';
        }
        (*os) << "{";
        auto it = projection_vars.cbegin();

        // This handles the case where the projection is empty
        if (it != projection_vars.cend()) {
            (*os) << "\"" << it->first << "\":" << (*child_iter)[it->second];
            while (++it != projection_vars.cend()) {
                (*os) << "," << "\"" << it->first << "\":" << (*child_iter)[it->second];
            }
        }
        (*os) << "}";
        count++;
        return true;
    } else {
        (*os) << "]}}";
        return false;
    }
}

GraphObject Select::operator[](VarId var) const {
    return (*child_iter)[var];
}

void Select::analyze(std::ostream& os, int indent) const {
    child_iter->analyze(os, indent);
    os << std::string(indent, ' ');
    os << "Select(";
    for (auto& pair : projection_vars) {
        os << " " << pair.first << "(" << pair.second.id << ")";
    }
    if (limit != OpSelect::DEFAULT_LIMIT) {
        os << " LIMIT " << limit;
    }
    os << " )\n";
}
