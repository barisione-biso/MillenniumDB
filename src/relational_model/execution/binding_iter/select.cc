#include "select.h"

#include "storage/index/object_file/object_file.h"
#include "relational_model/execution/binding_id_iter/property_paths/path_manager.h"

using namespace std;

Select::Select(unique_ptr<BindingIter> _child_iter,
               vector<pair<Var, VarId>> projection_vars,
               uint_fast32_t limit) :
    child_iter      (move(_child_iter)),
    limit           (limit),
    my_binding      (BindingSelect(move(projection_vars), child_iter->get_binding()) ) { }


Select::~Select() {
    // TODO: Move clear
    path_manager.clear();
}


void Select::begin() {
    child_iter->begin();
}


bool Select::next() {
    if (limit != 0 && count >= limit) {
        return false;
    }
    if (child_iter->next()) {
        count++;
        return true;
    } else {
        return false;
    }
}


void Select::analyze(int indent) const {
    child_iter->analyze(indent);
}
