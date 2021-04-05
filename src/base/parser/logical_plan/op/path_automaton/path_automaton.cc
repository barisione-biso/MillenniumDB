#include "path_automaton.h"

#include <iostream>
#include <queue>
#include <stack>

using namespace std;

/*
Supuestos importantes del automata:
    - El estado inicial siempre será uno.
    - Desde start siempre se va a poder llegar todos los states de end
    - Crear un ciclo, ya sea de un nodo hacia si mismo o hacia otro puede
      cambiar la semántica del automata. Al momento de optimizar se revisan los
      casos:
        - No se puede mergear 2 nodos que hacen un 2-ciclo (Al final se mergea el nodo consigo mismo)
        - Al reemplazar una transicion epsilon desde v hacia s, v != s
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
    s y v se pueden fusionar: No es condicion suficiente, se deben revisar ciclos
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


void PathAutomaton::optimize_automata() {
    print();
    cout << "---------------------\n";
    delete_mergeable_states();
    print();
    cout << "================\n";
    // Por cada transición epsilon del autómata de la forma (a)=[]=>(b):
    // calcular clausura epsilon de (b) si aún no se ha hecho
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
    // TODO: Revisar si la clausura se calcula de a o de b
    for (size_t a = 0; a < from_to_connections.size(); a++) {
        auto epsilon_closure = get_epsilon_closure(a);
        for (const auto s : epsilon_closure) {
            // si (s) es final, marcar (a) como final
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
    delete_absortion_states();
}


void PathAutomaton::delete_mergeable_states() {
    bool has_changes = true;
    while (has_changes) {
        has_changes = false;
        /*
        Nota importante: El caso en que from y s sean = 0 implica
        que el automata es de la forma 0=[]=>0. En tal caso el resultado
        será un autómata vacío, que para efectos prácticos realiza lo mismo
        que el anterior, no acepta nada o acepta todo dependiendo si s es final.
        Se añade una condicion adicional para el merge: los estados a mergear deben ser distintos,
        no tiene sentido mergear un nodo consigo mismo.

        Las condiciones de merge de una sola transicion epsilon, es necesaria, pero no es suficiente.
        Adicionalmente se debe cumplir que los estados a mergear no formen un ciclo, o dicho de otra manera
        que no haga un merge de un nodo consigo mismo, porque eso puede hacer que se pierdan transiciones
        no necesariamente vacías o cambiar el significado del autómata
        - s y v se mergean con v final, s pasa a ser final (o mergeo hacia v): Listo
        - s y v se mergean con s = start, mergeo v hacia s: Listo
        - s y v se mergean y hacen ciclo de largo 2 Se debe omitir este merge: Listo  
        - s y v se mergean y son dos estados cualquiera: Listo
        */
        for (size_t s = 0; s < from_to_connections.size(); s++) {
            // If s only can by reached from v and the transition is epsilon, then v = s
            if (to_from_connections[s].size() == 1 &&
                to_from_connections[s][0].label.empty() &&
                !has_two_length_cycle(s))
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
            if (from_to_connections[s].size() == 1 &&
                from_to_connections[s][0].label.empty() &&
                !has_two_length_cycle(s))
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


void PathAutomaton::merge_states(uint32_t destiny, uint32_t source) {
    /*
    En un merge de v hacia s: Todas las conexiones que salen de v, ahora saldran
    de s. Todas las conexiones que se dirijan a v, se dirigen ahora a s. Se maneja el
    caso en que v sale hacia si mismo, donde luego s sale de si mismo
    */
    if (end.find(source) != end.end()) {
        end.insert(destiny);
    }
    // Redirect source=[x]=>v to destiny=[x]=>v
    for (const auto& t : from_to_connections[source]) {
        if (t.from == t.to) {
            connect(Transition(destiny, destiny, t.label, t.inverse));
        } else if (t.to != destiny) {
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
        } else if (t.from != destiny) {
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


bool PathAutomaton::has_two_length_cycle(uint32_t state) {
    for (const auto& from_transitions : from_to_connections[state]) {
        for (const auto& to_transitions : to_from_connections[state]) {
            if (from_transitions.to == to_transitions.from) {
                return true;
            }
        }
    }
    return false;
}
