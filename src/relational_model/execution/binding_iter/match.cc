#include "match.h"

#include <limits>

#include "storage/index/object_file/object_file.h"

using namespace std;

Match::Match(const GraphModel& model, unique_ptr<BindingIdIter> root, size_t binding_size) :
    model      (model),
    root       (move(root)),
    input      (binding_size),
    my_binding (BindingMaterializeId(model, binding_size, input)) { }


void Match::begin() {
    root->begin(input, true);
}


bool Match::next() {
    return root->next();
}


void Match::analyze(int indent) const {
    root->analyze(indent);
}
