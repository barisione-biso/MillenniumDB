#include "binding_order_by.h"

#include <iostream>

#include "base/binding/binding.h"
#include "base/graph/value/value.h"

using namespace std;

BindingOrderBy::BindingOrderBy(vector<pair<string, VarId>> projection_vars, Binding& child_binding) :
    projection_vars (move(projection_vars)),
    child_binding   (child_binding) { }


std::string BindingOrderBy::to_string() const {
    std::string result;
    result += '{';
    auto it = projection_vars.cbegin();

    while (true) {
        auto& var_varid_pair = *it;
        result += var_varid_pair.first;
        result += ':';
        result += child_binding[var_varid_pair.second]->to_string();
        ++it;
        if (it != projection_vars.cend()) {
            result += ',';
        } else {
            result +=  "}\n";
            return result;
        }
    }
}


shared_ptr<GraphObject> BindingOrderBy::operator[](const VarId var) {
    return child_binding[var];
}
