#include "binding_project.h"

#include <iostream>

#include "base/binding/binding.h"
#include "base/graph/value/value.h"

using namespace std;

BindingProject::BindingProject(vector<string>& projection_vars, unique_ptr<Binding> current_binding)
    : projection_vars(projection_vars), current_binding(move(current_binding)) { }


std::string BindingProject::to_string() const {
    std::string result;
    result += '{';
    auto it = projection_vars.cbegin();

    while (true) {
        auto& var = *it;
        result += var;
        result += ':';
        result += (*current_binding)[var]->to_string();
        ++it;
        if (it != projection_vars.cend()) {
            result += ',';
        } else {
            result +=  "}\n";
            return result;
        }
    }
}


shared_ptr<GraphObject> BindingProject::operator[](const string& var) {
    for (auto& projected_var : projection_vars) {
        if (projected_var == var) {
            return (*current_binding)[var];
        }
    }
    return nullptr;
}


shared_ptr<GraphObject> BindingProject::get(const string& var, const string& key) {
    return (*this)[var + "." + key];
}