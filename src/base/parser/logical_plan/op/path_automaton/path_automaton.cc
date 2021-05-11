#include "path_automaton.h"

#include <iostream>
#include <queue>
#include <stack>
#include <utility>

using namespace std;


PathAutomaton::PathAutomaton() { }


void PathAutomaton::print() {
     for (size_t i = 0; i < from_to_connections.size(); i++) {
        for (auto& t : from_to_connections[i]) {
            cout << t.from << "=[" << (t.inverse ? "^" : "") << t.label << "]=>" << t.to << "\n";
        }
    }
    cout << "distance to end: \n";
    for (size_t i = 0; i < distance_to_final.size(); i++) {
        cout << i << ":" << distance_to_final[i] << "\n";
    }
    cout << "start is final: " << (start_is_final ? "true" : "false") << "\n";
    cout << "end states: { ";
    for (auto& state : end) {
        cout << state << "  ";
    }
    cout << "}\n";
    cout << "final state: " << final_state << "\n";
}


void PathAutomaton::rename_and_merge(PathAutomaton& other) {
    // Add and rename 'other' states to this automaton
    for (size_t i = 0; i < other.from_to_connections.size(); i++) {
        auto new_connection = vector<Transition>();
        for (auto& t : other.from_to_connections[i]) {
            auto transition = Transition(t.from + total_states, t.to + total_states, t.label, t.inverse);
            connect(transition);
        }
    }

    // Rename 'other' start and end states
    set<uint32_t> new_end;
    for (auto& end_state : other.end) {
        new_end.insert(total_states + end_state);
    }
    other.start += total_states;
    other.end = move(new_end);

    // Add 'other' states count to this automaton
    total_states += other.total_states;
}


void PathAutomaton::connect(Transition transition) {
    // Check if connections vector has slots to save from and to
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

    // Check if a the connections exists
    bool exists = false;
    for (auto& t : from_to_connections[transition.from]) {
        if (transition == t) {
            exists = true;
            break;
        }
    }

    // Add connection if it doesn't exists
    if (!exists) {
        from_to_connections[transition.from].push_back(transition);
        to_from_connections[transition.to].push_back(transition);
    }
}


void PathAutomaton::add_epsilon_transition(uint32_t from, uint32_t to) {
    connect(Transition(from , to, "", false));
}


void PathAutomaton::optimize_automata() {
    delete_mergeable_states();
    // Por cada transición epsilon del autómata de la forma (a)=[]=>(b):
    // calcular clausura epsilon de (b) si aún no se ha hecho
    // por cada estado (s) en la clausura epsilon de (b):
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
    // TODO: Revisar si la clausura se calcula de a o de b
    for (size_t a = 0; a < from_to_connections.size(); a++) {
        auto epsilon_closure = get_epsilon_closure(a);
        for (const auto s : epsilon_closure) {
            if (end.find(s) != end.end()) {
                end.insert(a);
            }
        }
        // TODO: use vector iterator for more readable code
        for (size_t i = 0; i < from_to_connections[a].size();) {
            // from_to_connections[a][i] transition: (a)=[]=>(b)
            if (from_to_connections[a][i].label.empty()) {
                for (const auto s : epsilon_closure) {
                    for (const auto& t : from_to_connections[s]) {
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
    if (end.size() == 1) {
        final_state = *end.begin();
        start_is_final = final_state == start;
    } else {
        set_final_state();
    }
    end.clear();
    delete_absortion_states();
    calculate_distance_to_final_state();
}


void PathAutomaton::delete_mergeable_states() {
    bool has_changes = true;
    while (has_changes) {
        has_changes = false;
        for (size_t s = 0; s < from_to_connections.size(); s++) {
            // If s only can by reached from v and the transition is epsilon, then v = s
            // from != s to avoid merge a state with itself
            if (to_from_connections[s].size() == 1 &&
                to_from_connections[s][0].label.empty() &&
                to_from_connections[s][0].from != s)
            {
                // If from = start, merge to from avoiding delete start
                if (to_from_connections[s][0].from == start) {
                    merge_states(to_from_connections[s][0].from, s);
                } else {
                    merge_states(s, to_from_connections[s][0].from);
                }

                has_changes = true;
            }
            // If v only has one transition to s, and it is epsilon, then s = v
            // to != s to avoid merge a state with itself
            if (from_to_connections[s].size() == 1 &&
                from_to_connections[s][0].label.empty() &&
                from_to_connections[s][0].to != s)
            {
                if (from_to_connections[s][0].to == start) {
                    merge_states(from_to_connections[s][0].to, s);
                } else {
                    merge_states(s, from_to_connections[s][0].to);
                }
                has_changes = true;
            }
        }
    }
}


set<uint32_t> PathAutomaton::get_epsilon_closure(uint32_t state) {
    // Automaton exploration is with dfs algorithm
    set<uint32_t>  epsilon_closure;
    // It is not necesary to force to state belong to it own epsilon closure
    set<uint32_t> visited;
    stack<uint32_t> open;
    open.push(state);
    uint32_t current_state;
    while (!open.empty()) {
        current_state = open.top();
        open.pop();
        if (visited.find(current_state) == visited.end()) {
            visited.insert(current_state);
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
    // Get reachable states from start state
    auto reachable_states = get_reachable_states_from_start();
    for (size_t i = 1; i < from_to_connections.size(); i++) {
        // Check if 'i0 is reachable from start
        if (reachable_states.find(i) == reachable_states.end()) {
            // Delete all transitions from and to 'i' if is not reachable
            from_to_connections[i].clear();
            to_from_connections[i].clear();
            end.erase(i);
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
    // Get state that can reach final_state
    auto end_reachable_states = get_reachable_states_from_end();
    for (size_t i = 0; i < from_to_connections.size(); i++) {
        // Check if 'i' can reach to end state
        if (end_reachable_states.find(i) == end_reachable_states.end()) {
            // Delete all 'i' transitions
            from_to_connections[i].clear();
            to_from_connections[i].clear();
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
    // DFS with initial point the start state
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
    // All reachable states will be in visited set
    return visited;
}


set<uint32_t> PathAutomaton::get_reachable_states_from_end() {
    // DFS with start point the final_state.
    stack<uint32_t> open;
    set<uint32_t> visited;
    uint32_t current_state;
    open.push(final_state);
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
    // All end' reachable states will be in visited set
    return visited;
}


void PathAutomaton::merge_states(uint32_t destiny, uint32_t source) {
    if (end.find(source) != end.end()) {
        end.insert(destiny);
    }
    // Redirect source=[x]=>v to destiny=[x]=>v
    for (const auto& t : from_to_connections[source]) {
        if (t.from == t.to) {
            connect(Transition(destiny, destiny, t.label, t.inverse));
        } else {
            connect(Transition(destiny, t.to, t.label, t.inverse));
        }
        // Delete source=[x]=>v
        auto to_iterator = to_from_connections[t.to].begin();
        while (to_iterator != to_from_connections[t.to].end()) {
            if ((*to_iterator) == t) {
                to_from_connections[t.to].erase(to_iterator);
                to_iterator = to_from_connections[t.to].end();
            } else {
                to_iterator++;
            }
        }
    }
    // Redirect v=[x]=>source to v=[x]=>destiny
    for (const auto& t : to_from_connections[source]) {
        if (t.from == t.to) {
            connect(Transition(destiny, destiny, t.label, t.inverse));
        } else {
            connect(Transition(t.from, destiny, t.label, t.inverse));
        }
        // Delete v=[x]=>source
        auto from_iterator = from_to_connections[t.from].begin();
        while (from_iterator != from_to_connections[t.from].end()) {
            if ((*from_iterator) == t) {
                from_to_connections[t.from].erase(from_iterator);
                from_iterator = from_to_connections[t.from].end();
            } else {
                from_iterator++;
            }
        }
    }
    from_to_connections[source].clear();
    to_from_connections[source].clear();
}

void PathAutomaton::set_final_state() {
    final_state = total_states;
    total_states++;
    // Set start state as final
    if (end.find(start) != end.end()) {
        start_is_final = true;
    }
    // Redirect state that reach to end's state to final_state

    for (size_t i = 0; i < from_to_connections.size(); i++) {
        for (size_t j = 0; j < from_to_connections[i].size(); j++) {
            const auto& t = from_to_connections[i][j];
            if (end.find(t.to) != end.end()) {
                connect(Transition(t.from, final_state, t.label, t.inverse));
            }
        }
    }
}

void PathAutomaton::calculate_distance_to_final_state() {
    // BFS with initial point the final_state
    queue<pair<uint32_t, uint32_t>> open;
    set<uint32_t> visited;
    for (uint32_t i = 0; i < total_states; i++) {
        distance_to_final.push_back(UINT32_MAX);
    }
    open.push(make_pair(final_state, 0));
    while (!open.empty()) {
        auto current_pair = open.front();
        auto current_state = current_pair.first;
        auto current_distance = current_pair.second;
        open.pop();
        if (visited.find(current_state) == visited.end()) {
            visited.insert(current_state);
            // Set distance to final_state. BFS guarantees that this distance
            // is the minimum
            distance_to_final[current_state] = current_distance;
            for (const auto& transition : to_from_connections[current_state]) {
                open.push(make_pair(transition.from, current_distance + 1));
            }
        }
    }
}