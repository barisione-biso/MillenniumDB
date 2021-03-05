#include "distinct.h"

using namespace std;

Distinct::Distinct(GraphModel& model, unique_ptr<BindingIter> _child_iter) :
    model      (model),
    child_iter (move(_child_iter)),
    my_binding (BindingDistinct(model)) { }


void Distinct::begin() {
    child_iter->begin();
}


bool Distinct::next() {
    while (child_iter->next()) {
        //my_binding.clear_cache();
        return true;
    }
    return false;
}


void Distinct::analyze(int indent) const {
    std::cout << "Distinct()\n";
    child_iter->analyze(indent+2);
}
