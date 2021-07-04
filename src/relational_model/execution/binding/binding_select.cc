#include "binding_select.h"

#include <iostream>

#include "base/binding/binding.h"

using namespace std;

BindingSelect::BindingSelect(vector<pair<string, VarId>> projection_vars, Binding& child_binding) :
    projection_vars (move(projection_vars)),
    child_binding   (child_binding) { }


std::ostream& BindingSelect::print_to_ostream(std::ostream& os) const {
    os << '{';
    auto it = projection_vars.cbegin();

    if (it != projection_vars.cend()) {
        auto& var_varid_pair = *it;
        os << var_varid_pair.first << ':' << child_binding[var_varid_pair.second];
        ++it;
    }
    while (it != projection_vars.cend()) {
        auto& var_varid_pair = *it;
        os << ", " << var_varid_pair.first << ':' << child_binding[var_varid_pair.second];
        ++it;
    }
    return os << "}";
}


GraphObject BindingSelect::operator[](const VarId var) {
    return child_binding[var];
}
