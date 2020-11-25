#include "select.h"

#include "storage/index/object_file/object_file.h"

using namespace std;

Select::Select(unique_ptr<BindingIter> _child_iter,
               vector<pair<string, VarId>> projection_vars,
               uint_fast32_t limit) :
    child_iter      (move(_child_iter)),
    limit           (limit),
    my_binding      (BindingSelect(move(projection_vars), child_iter->get_binding()) ) { }


Select::~Select() = default;


Binding& Select::get_binding() {
    return my_binding;
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
