#include "union.h"

using namespace std;

Union::Union(std::size_t binding_size, std::vector<std::unique_ptr<BindingIdIter>> iters) :
    BindingIdIter(binding_size),
    iters(move(iters)) { }


BindingId& Union::begin(BindingId& input) {
    current_iter = 0;
    for (auto& iter : iters) {
        my_inputs.push_back( &iter->begin(input) );
    }
    return my_binding;
}


void Union::reset() {
    current_iter = 0;
    for (auto& iter : iters) {
        iter->reset();
    }
}


bool Union::next() {
    while (current_iter < iters.size()) {
        if (iters[current_iter]->next()) {
            my_binding.add_all(*my_inputs[current_iter]);
            return true;
        } else {
            ++current_iter;
        }
    }
    return false;
}


void Union::analyze(int indent) const {
    cout << "Union:\n";
    for (const auto& iter : iters) {
        iter->analyze(indent + 2);
    }
}
