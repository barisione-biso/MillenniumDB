#include "union.h"

using namespace std;

Union::Union(std::vector<std::unique_ptr<BindingIdIter>> iters) :
    iters(move(iters)) { }


BindingId* Union::begin(BindingId& input) {
    my_binding = make_unique<BindingId>(input.var_count());
    current_iter = 0;
    for (auto& iter : iters) {
        my_inputs.push_back( iter->begin(input) );
    }
    return my_binding.get();
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
            my_binding->add_all(*my_inputs[current_iter]);
            return true;
        } else {
            ++current_iter;
        }
    }
    return false;
}


void Union::analyze(int indent) const {
    cout << "Sequence:\n";
    for (const auto& iter : iters) {
        iter->analyze(indent + 2);
        cout << "\n";
    }
}
