#include "binding_root.h"

#include <iostream>

using namespace std;

BindingRoot::BindingRoot() { }


void BindingRoot::print() const {
    cout << "{}\n";
}


shared_ptr<Value> BindingRoot::operator[](const string& ) {
    return nullptr;
}

shared_ptr<Value> BindingRoot::try_extend(const string& , const string& ) {
    return nullptr;
}
