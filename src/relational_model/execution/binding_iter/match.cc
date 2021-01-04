#include "match.h"

#include <limits>

#include "storage/index/object_file/object_file.h"

using namespace std;

Match::Match(GraphModel& model, unique_ptr<BindingIdIter> _root, size_t binding_size) :
    model      (model),
    root       (move(_root)),
    input      (binding_size),
    my_binding (BindingMaterializeId(model, binding_size, input/*root->begin(input)*/) )
{
    root->begin(input, true); // TODO: hacer en otra parte? BindingMaterializeId?
}


Binding& Match::get_binding() {
    return my_binding;
}


bool Match::next() {
    return root->next();
}


void Match::analyze(int indent) const {
    root->analyze(indent);
}
