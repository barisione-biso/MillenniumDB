#include "projection.h"

#include "file/index/object_file/object_file.h"
#include "relational_model/binding/binding_project.h"
#include "relational_model/binding/binding_root.h"

using namespace std;

Projection::Projection(unique_ptr<BindingIter> iter, set<string> projection_vars)
    : iter(move(iter)), projection_vars(move(projection_vars)) { }


void Projection::begin() {
    // root_input = make_unique<BindingRoot>();
    iter->begin(/**root_input*/);
}


std::unique_ptr<Binding> Projection::next() {
    auto next_binding = iter->next();
    if (next_binding == nullptr) {
        return nullptr;
    }
    else {
        return make_unique<BindingProject>(projection_vars, move(next_binding));;
    }
}
