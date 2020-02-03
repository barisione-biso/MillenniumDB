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
