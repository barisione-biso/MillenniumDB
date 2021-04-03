#include "path_automaton.h"

#include <iostream>
#include <queue>
#include <stack>

using namespace std;


/*
Recorrer y encontrar:
    - Estados a los que no se pueda llegar
    - Estados que no lleguen al nodo final
Se deben eliminar (su connections[i] debe quedar vacio)
Ser generico, que funcione para cualquier automata, no necesariamente
construido con las reglas del generado actualmente

- Revisar condicion de merge al momento de unir dos estados, antes de optimizarlo
    - Condicion: A s solo llega un epsilon desde v, o solo sale un epsilon hacia v. Entonce
    s y v se pueden fusionar
- Evaluar manera de guardar conexiones: segundo vector que guarde to <- from
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


void PathAutomaton::rename_and_merge(PathAutomaton& other) {
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
    while (connections.size() <= transition.from || connections.size() <= transition.to) {
        vector<Transition> new_vec;
        connections.push_back(new_vec);
    }
    while (incidence_vector.size() <= transition.from || incidence_vector.size() <= transition.to) {
        incidence_vector.push_back(0);
    }
    bool exists = false;
    for (auto& t : connections[transition.from]) {
        if (transition == t) {
            exists = true;
            break;
        }
    }
    if (!exists) {
        connections[transition.from].push_back(transition);
        incidence_vector[transition.to]++;
    }
}


void PathAutomaton::add_epsilon_transition(uint32_t from, uint32_t to) {
    connect(Transition(from , to, "", false));
}


void PathAutomaton::merge_empty_states() {
    //TODO: Generalizar para caminos de largo mayor a 1
    for (auto& transitions : connections) {
        for (auto& transition : transitions) {
            if (transition.label.empty()
                && ((transition.to >= connections.size()) || connections[transition.to].empty())
                && end.find(transition.to) != end.end())
            {
                end.insert(transition.from);
            }
        }
    }
}


void PathAutomaton::optimize_automata() {
    print();
    cout << "================\n";
    // for (size_t s = 0; s < connections.size(); s++) {
    //     auto epsilon_closure = get_epsilon_closure(s);
    //     for (const auto& v : epsilon_closure) {
    //         // If closure state v is final, then state s is final
    //         if (end.find(v) != end.end()) {
    //             end.insert(s);
    //         }
    //         for (const auto& t : connections[v]) {
    //             if (!t.label.empty()) {
    //                 connect(Transition(s, t.to, t.label, t.inverse));
    //             }
    //         }
    //     }
    //     //clean_automata(s);
    // }
    // Por cada transición epsilon del autómata de la forma (a)=[]=>(b):
    // calcular clausura epsilon de (a) si aún no se ha hecho
    // por cada estado (s) en la clausura epsilon de (a):
    //     si (s) es final, marcar (a) como final
    //     por cada transición desde (s), de la forma (s)=[l]=>(t):
    //         crear transicion (a)=[l]=>(t)
    //         // la transicion (s)=[l]=>(t) no se borra
    // se borra la transicion epsilon (a)=[]=>(b)

    // Ahora que se eliminaron todas las transiciones epsilon se pueden eliminar los estados
    // que no son alcanzables desde el estado inicial y los estados de los cuales no puedo llegar
    // a un estado final
    // Esto se hace con 2 pasadas DFS/BFS visitando nodos y marcandolos.
    // Primero una pasada partiendo en el estado inicial y llendo hacia adelante.
    // Luego otra pasada partierndo en todos los estados finales y llendo hacia atras.
    // Se eliminan las transiciones que salen o llegan a los nodos no visitados.
    for (size_t a = 0; a < connections.size(); a++) {
        auto epsilon_closure = get_epsilon_closure(a);
        for (const auto s : epsilon_closure) {
            // si (s) es final, marcar (a) como final
            if (end.find(s) != end.end()) {
                end.insert(a);
            }
        }

        // TODO: use vector iterator for more readable code
        for (size_t i = 0; i < connections[a].size(); /*i++ only when connection is not deleted*/) { // connections[a][i] transition: (a)=[]=>(b)
            if (connections[a][i].label.empty()) {
                for (const auto s : epsilon_closure) {
                    for (const auto& t : connections[s]) { // (s)=[l]=>(t)
                        if (!t.label.empty()) {
                            connect(Transition(a, t.to, t.label, t.inverse));
                        }
                    }
                }
                // delete epsilon transition
                connections[a].erase(connections[a].begin() + i);
            } else {
                i++;
            }
        }
    }


    /*
    print();
    cout << "===============\n";
    merge_empty_states();
    for (size_t i = 0; i < connections.size(); i++) {
        vector<Transition> new_transitions;
        for (size_t j = 0; j < connections[i].size(); j++) {
            if (connections[i][j].label.empty()) {
                delete_epsilon_from(connections[i][j]);
            }
        }
        for (auto& t : connections[i]) {
            if (!t.label.empty()) {
                new_transitions.push_back(t);
            }
        }
        connections[i] = new_transitions;
        delete_unreachable_states();
    }
    */
}


void PathAutomaton::delete_epsilon_from(Transition transition) {
    // Bfs from transition.to

    queue<uint32_t> open;
    open.push(transition.to);
    set<uint32_t> visited;
    uint32_t current_state;
    set<uint32_t> states_in_next_deep;
    states_in_next_deep.insert(transition.to);
    bool stop = false;
    while (!open.empty()) {
        current_state = open.front();
        if (stop && states_in_next_deep.size() == 0) {
            return;
        }
        states_in_next_deep.erase(current_state);
        open.pop();
        if (visited.find(current_state) == visited.end()) {
            visited.insert(current_state);
            for (auto& t : connections[current_state]) {
                if (!t.label.empty()) {
                    connect(Transition(transition.from, t.to, t.label, t.inverse));
                    stop = true;
                } else if (!stop) {
                    open.push(t.to);
                    states_in_next_deep.insert(t.to);
                }
            }
        }
    }
}


void PathAutomaton::delete_unreachable_states() {
    set<uint32_t> reachable_states;
    reachable_states.insert(0);
    for (auto& states : connections) {
        for (auto& transition : states) {
            reachable_states.insert(transition.to);
        }
    }
    for (size_t i = 0; i < connections.size(); i++) {
        if (reachable_states.find(i) == reachable_states.end()) {
            vector<Transition> empty_state;
            connections[i] = empty_state;
        }
    }
}


set<uint32_t> PathAutomaton::get_epsilon_closure(uint32_t state) {
    //Dfs from state
    set<uint32_t>  epsilon_closure;
    set<uint32_t> visited;
    stack<uint32_t> open;
    open.push(state);
    //epsilon_closure.insert(state);
    uint32_t current_state;
    while (!open.empty()) {
        current_state = open.top();
        open.pop();
        if (visited.find(current_state) == visited.end()) {
            for (auto& transition : connections[current_state]) {
                if (transition.label.empty()) {
                    epsilon_closure.insert(transition.to);
                    open.push(transition.to);
                }
            }
        }
    }
    return epsilon_closure;
}

void PathAutomaton::clean_automata(uint32_t state) {
    vector<Transition> new_transitions;
    for (auto& t : connections[state]) {
        if (!t.label.empty()) {
            new_transitions.push_back(t);
        } else if (t.to != start) {
            if (incidence_vector[t.to] == 1) {
                vector<Transition> no_transitions;
                connections[t.to] = no_transitions;
            } else if (incidence_vector[t.to] >= 1){
                incidence_vector[t.to]--;
            }
        }
    }
    connections[state] = new_transitions;
}
