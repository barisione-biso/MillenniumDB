#include "binding_distinct.h"

#include <cassert>

#include "base/binding/binding.h"

using namespace std;


BindingDistinct::BindingDistinct(GraphModel& model, Binding& child_binding) :
    model         (model),
    child_binding (child_binding) { }


std::ostream& BindingDistinct::print_to_ostream(std::ostream& os) const {
    // TODO:
    return os;
}


GraphObject BindingDistinct::operator[](const VarId var) {
    return child_binding[var];
}
