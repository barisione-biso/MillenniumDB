#include "path_automaton.h"

#include <iostream>

using namespace std;

//TODO: Añadir epsilon transition (metodo)
/*TODO: Calcular clausuras:
    - Recorrer automata, para cada transicion epsilon, buscar el primer label no nulo, y conectar
    desde donde nace la transición con el estado que conecta el label.
*/


PathAutomaton::PathAutomaton() { }

void PathAutomaton::print() {
     for (size_t i = 0; i < connections.size(); i++) {
        for (auto& t : connections[i]) {
            cout << t.from << "=[" << (t.inverse ? "^" : "") << t.label << "]=>" << t.to << "\n";
        }
    }
    cout << "end states: { ";
    for (auto& state : end) {
        cout << state << "  ";
    }
    cout << "}\n";
}


void PathAutomaton::merge_with_automaton(PathAutomaton& other) {
    for (size_t i = 0; i < other.connections.size(); i++) {
        auto new_connection = vector<Transition>();
        for (auto& t : other.connections[i]) {
            auto transition = Transition(t.from + total_states, t.to + total_states, t.label, t.inverse);
            connect(transition);
        }
    }
    set<uint32_t> new_end;
    for (auto& end_state : other.end) {
        new_end.insert(total_states + end_state);
    }
    other.start += total_states;
    other.end = move(new_end);
    total_states += other.total_states;
}

void PathAutomaton::connect(Transition transition) {
    while (connections.size() <= transition.from) {
        vector<Transition> new_vec;
        connections.push_back(new_vec);
    }
    connections[transition.from].push_back(transition);
}

void PathAutomaton::add_epsilon_transition(uint32_t from, uint32_t to) {
    connect(Transition(from , to, "", false));
}
