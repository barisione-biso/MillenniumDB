#include "binding.h"

#include <iostream>

using namespace std;

Binding::Binding() {

}


int_fast32_t Binding::var_count(){
    return dict.size();
}


void Binding::add(string name, string value) {
    dict.insert({name, value});
}


void Binding::print() const {
    for (auto&&[key, value] : dict) {
        cout << key << ": " << value << "\t";
    }
    cout << "\n";
}
