#include "match.h"

#include <limits>

#include "storage/index/object_file/object_file.h"
#include "relational_model/execution/binding/binding_match.h"

using namespace std;

Match::Match(GraphModel& model, unique_ptr<BindingIdIter> root, map<string, VarId> var_pos) :
    model   (model),
    root    (move(root)),
    var_pos (move(var_pos)) { }


// TODO: devolver Binding*?
void Match::begin() {
    input.init(var_pos.size());
    binding_id_root = &root->begin(input);
}


unique_ptr<Binding> Match::next() {
    if (root->next()) {
        return make_unique<BindingMatch>(model, var_pos, binding_id_root);
    } else {
        return nullptr;
    }
}


void Match::analyze(int indent) const {
    root->analyze(indent);
}
