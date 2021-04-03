#include "path_automaton.h"

#include <iostream>
#include <queue>
#include <stack>

using namespace std;

/*
Supuestos importantes del automata:
    - El estado inicial siempre será uno.
    - Desde start siempre se va a poder llegar todos los states de end
*/


/*
Recorrer y encontrar:
    - Estados a los que no se pueda llegar: Listo
    - Estados que no lleguen al nodo final: Listo
Se deben eliminar (su from_to_connections[i] debe quedar vacio)
Ser generico, que funcione para cualquier automata, no necesariamente
construido con las reglas del generado actualmente

- Revisar condicion de merge al momento de unir dos estados, antes de optimizarlo
    - Condicion: A s solo llega un epsilon desde v, o solo sale un epsilon hacia v. Entonce
    s y v se pueden fusionar
*/

PathAutomaton::PathAutomaton() { }

void PathAutomaton::print() {
     for (size_t i = 0; i < from_to_connections.size(); i++) {
        for (auto& t : from_to_connections[i]) {
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
    for (size_t i = 0; i < other.from_to_connections.size(); i++) {
        auto new_connection = vector<Transition>();
        for (auto& t : other.from_to_connections[i]) {
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
    while (from_to_connections.size() <= transition.from ||
           from_to_connections.size() <= transition.to)
        {
            vector<Transition> new_vec;
            from_to_connections.push_back(new_vec);
        }
    while (to_from_connections.size() <= transition.from ||
           to_from_connections.size() <= transition.to)
        {
            vector<Transition> new_vec;
            to_from_connections.push_back(new_vec);
        }
    bool exists = false;
    for (auto& t : from_to_connections[transition.from]) {
        if (transition == t) {
            exists = true;
            break;
        }
    }
    if (!exists) {
        from_to_connections[transition.from].push_back(transition);
        to_from_connections[transition.to].push_back(transition);
    }
}


void PathAutomaton::add_epsilon_transition(uint32_t from, uint32_t to) {
    connect(Transition(from , to, "", false));
}


void PathAutomaton::merge_states() {
    for (size_t i = 0; i < from_to_connections.size(); i++) {
        // If v has only one transitions (and it is epsilon) to s, then v = s
        if (to_from_connections[i].size() == 1 && to_from_connections[i][0].label.empty()) {
            auto merge_state = to_from_connections[i][0].from;
            auto delete_state = to_from_connections[i][0].to;
            to_from_connections[delete_state].clear();
            if (end.find(delete_state) != end.end()) {
                end.insert(merge_state);
            }
            auto iterator = from_to_connections[merge_state].begin();
            while (iterator != from_to_connections[merge_state].end()) {
                if (iterator->to == delete_state && iterator->label.empty()) {
                    from_to_connections[merge_state].erase(iterator);
                } else {
                    iterator++;
                }
            }
            for (auto& old_t: from_to_connections[delete_state]) {
                connect(Transition(merge_state, old_t.to, old_t.label, old_t.inverse));
                auto iterator = to_from_connections[old_t.to].begin();
                while (iterator != to_from_connections[old_t.to].end()) {
                    if (iterator->from == delete_state) {
                        to_from_connections[old_t.to].erase(iterator);
                    } else {
                        iterator++;
                    }
                }
            }
            from_to_connections[delete_state].clear();
        }
    }
}


void PathAutomaton::optimize_automata() {
    print();
    cout << "---------------------\n";
    merge_states();
    print();
    cout << "================\n";
    // for (size_t s = 0; s < from_to_connections.size(); s++) {
    //     auto epsilon_closure = get_epsilon_closure(s);
    //     for (const auto& v : epsilon_closure) {
    //         // If closure state v is final, then state s is final
    //         if (end.find(v) != end.end()) {
    //             end.insert(s);
    //         }
    //         for (const auto& t : from_to_connections[v]) {
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
    for (size_t a = 0; a < from_to_connections.size(); a++) {
        auto epsilon_closure = get_epsilon_closure(a);
        for (const auto s : epsilon_closure) {
            // si (s) es final, marcar (a) como final
            if (end.find(s) != end.end()) {
                end.insert(a);
            }
        }

        // TODO: use vector iterator for more readable code
        for (size_t i = 0; i < from_to_connections[a].size(); /*i++ only when connection is not deleted*/) {
            // from_to_connections[a][i] transition: (a)=[]=>(b)
            if (from_to_connections[a][i].label.empty()) {
                for (const auto s : epsilon_closure) {
                    for (const auto& t : from_to_connections[s]) { // (s)=[l]=>(t)
                        if (!t.label.empty()) {
                            connect(Transition(a, t.to, t.label, t.inverse));
                        }
                    }
                }
                // delete epsilon transition
                from_to_connections[a].erase(from_to_connections[a].begin() + i);
            } else {
                i++;
            }
        }
    }
    delete_unreachable_states();
    //delete_absortion_states();
}



set<uint32_t> PathAutomaton::get_epsilon_closure(uint32_t state) {
    //Dfs from state
    set<uint32_t>  epsilon_closure;
    set<uint32_t> visited;
    stack<uint32_t> open;
    open.push(state);
    uint32_t current_state;
    while (!open.empty()) {
        current_state = open.top();
        open.pop();
        if (visited.find(current_state) == visited.end()) {
            for (auto& transition : from_to_connections[current_state]) {
                if (transition.label.empty()) {
                    epsilon_closure.insert(transition.to);
                    open.push(transition.to);
                }
            }
        }
    }
    return epsilon_closure;
}


void PathAutomaton::delete_unreachable_states() {
    /*
    Como se detectan estados incalzables:
    Se ejecuta DFS desde start hasta explorar todo el grafo, de este
    modo se visitan todos los nodos que se pueden alcanzar desde start
    y se retorna el conjunto de estos. Luego se recorren los estados y se vacian
    los vectores de aquellos estados que no estén en el conjunto
    */
    auto reachable_states = get_reachable_states_from_start();
    for (size_t i = 1; i < from_to_connections.size(); i++) {
        if (reachable_states.find(i) == reachable_states.end()) {
            from_to_connections[i].clear();
            to_from_connections[i].clear();
            /*
            Nota importante:
            La siguiente iteración revisa todos las transiciones que iban
            al estado i. Como se borra i también se borra esa transicion.
            Pero puede que esta iteracion sea innecesaria:
            Si desde start no puede llegar al estado v. Y el estado s
            conecta con v -> s es inalcanzable desde start (s seria explorado
            en el DFS desde start).
            Esto es independiente de las reglas para elaborar el automata.
            */
            for (size_t j = 0; j < from_to_connections.size(); j++) {
                auto iterator = from_to_connections[j].begin();
                while (iterator != from_to_connections[j].end()) {
                    if (iterator->to == i) {
                        from_to_connections[j].erase(iterator);
                    } else {
                        iterator++;
                    }
                }
                iterator = to_from_connections[j].begin();
                while (iterator != to_from_connections[j].end()) {
                    if (iterator->from == i) {
                        to_from_connections[j].erase(iterator);
                    } else {
                        iterator++;
                    }
                }
            }
        }
    }
}

void PathAutomaton::delete_absortion_states() {
    // Se usa la idea anterior pero haciendo dfs desde cada estado
    // que es de end
    auto end_reachable_states = get_reachable_states_from_end();
    for (size_t i = 0; i < from_to_connections.size(); i++) {
        if (end_reachable_states.find(i) == end_reachable_states.end()) {
            from_to_connections[i].clear();
            to_from_connections[i].clear();
            // Mismo caso anterior
            for (size_t j = 0; j < from_to_connections.size(); j++) {
                auto iterator = from_to_connections[j].begin();
                while (iterator != from_to_connections[j].end()) {
                    if (iterator->to == i) {
                        from_to_connections[j].erase(iterator);
                    } else {
                        iterator++;
                    }
                }
                iterator = to_from_connections[j].begin();
                while (iterator != to_from_connections[j].end()) {
                    if (iterator->from == i) {
                        to_from_connections[j].erase(iterator);
                    } else {
                        iterator++;
                    }
                }
            }
        }
    }
}


set<uint32_t> PathAutomaton::get_reachable_states_from_start() {
    stack<uint32_t> open;
    set<uint32_t> visited;
    open.push(start);
    uint32_t current_state;
    while (!open.empty()) {
        current_state = open.top();
        open.pop();
        if (visited.find(current_state) == visited.end()) {
            visited.insert(current_state);
            for (const auto& transition: from_to_connections[current_state]) {
                open.push(transition.to);
            }
        }
    }
    return visited;
}

set<uint32_t> PathAutomaton::get_reachable_states_from_end() {
    stack<uint32_t> open;
    set<uint32_t> visited;
    uint32_t current_state;
    for (const auto& end_state : end) {
        open.push(end_state);
        while (!open.empty()) {
            current_state = open.top();
            open.pop();
            if (visited.find(current_state) == visited.end()) {
                visited.insert(current_state);
                for (const auto& transition : to_from_connections[current_state]) {
                    open.push(transition.from);
                }
            }
        }
    }
    return visited;
}
