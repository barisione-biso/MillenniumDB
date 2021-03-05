#include "binding_distinct.h"

#include <cassert>

#include "base/binding/binding.h"

using namespace std;


BindingDistinct::BindingDistinct(GraphModel& model) :
    model         (model) { }


std::ostream& BindingDistinct::print_to_ostream(std::ostream& os) const {
    return os;
}


GraphObject BindingDistinct::operator[](const VarId var) {
    //TODO: change this and add assert like this one assert(var.id < binding_size);
    return objects_vector[var.id];
}

