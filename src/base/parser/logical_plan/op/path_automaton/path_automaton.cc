#include "path_automaton.h"

#include <iostream>

using namespace std;

uint32_t PathAutomaton::id = 0;

//TODO: Como manejar el id path automaton
/*TODO: Simplificar automata:
    - Al momento de crearlo, en alternatives hay una simplificacion
    - Merge de paths vacios  x1=[]=>x2, se puede hacer cuando:
        - x1 solo se conecta a x2, x2 se puede conectar a más de uno
    - Simplificaciones post calculo de clausuras
*/
//TODO: Calcular clausuras


PathAutomaton::PathAutomaton() {
    start = PathAutomaton::id;
    PathAutomaton::id++;
    end = PathAutomaton::id;
    PathAutomaton::id++;
}

void PathAutomaton::print() {
     for (auto i = conections.begin(); i != conections.end(); ++i) {
        for (auto& j : i->second) {
            cout << i->first << "=[" << (std::get<2>(j) ? "^" : "") << std::get<1>(j) << "]=>" << std::get<0>(j) << "\n";
        }
    }

}


void PathAutomaton::merge_with_automaton(PathAutomaton automaton) {
    //conections.insert(automaton.conections.begin(), automaton.conections.end());
    conections.merge(automaton.conections);
}

void PathAutomaton::connect_states(uint32_t from, uint32_t to, std::string type, bool inverse) {
    if (conections.find(from) == conections.end()) {
        vector<tuple<uint32_t, std::string, bool>> new_vec;
        new_vec.push_back(make_tuple(to, type, inverse));
        conections[from] = new_vec;
    } else {
        conections[from].push_back(make_tuple(to, type, inverse));
    }
}
