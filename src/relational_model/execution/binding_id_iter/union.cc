#include "union.h"

using namespace std;

Union::Union(std::vector<std::unique_ptr<BindingIdIter>> iters) :
    iters(move(iters)) { }


void Union::begin(BindingId& input) {
    current_iter = 0;
    for (auto& iter : iters) {
        iter->begin(input);
    }
}


void Union::reset(BindingId& input) {
    current_iter = 0;
    for (auto& iter : iters) {
        iter->reset(input);
    }
}


BindingId* Union::next() {
    while (current_iter < iters.size()) {
        auto res = iters[current_iter]->next();
        if (res != nullptr) {
            return res;
        } else {
            ++current_iter;
        }
    }
    return nullptr;
}


void Union::analyze(int indent) const {
    cout << "Sequence:\n";
    for (const auto& iter : iters) {
        iter->analyze(indent + 2);
        cout << "\n";
    }
}
