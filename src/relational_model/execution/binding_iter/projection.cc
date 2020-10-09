#include "projection.h"

#include "storage/index/object_file/object_file.h"
#include "relational_model/execution/binding/binding_project.h"

using namespace std;

Projection::Projection(unique_ptr<BindingIter> iter, vector<string> projection_vars, uint_fast32_t limit) :
    projection_vars (move(projection_vars)),
    iter            (move(iter)),
    limit           (limit) { }

Projection::~Projection() = default;


void Projection::begin() {
    count = 0;
    iter->begin();
}


std::unique_ptr<Binding> Projection::next() {
    if (limit != 0 && count >= limit) {
        return nullptr;
    }
    auto next_binding = iter->next();
    if (next_binding == nullptr) {
        return nullptr;
    } else {
        count++;
        if (projection_vars.size() == 0) {
            return next_binding;
        } else {
            return make_unique<BindingProject>(projection_vars, move(next_binding));
        }
    }
}


void Projection::analyze(int indent) const {
    iter->analyze(indent);
}
