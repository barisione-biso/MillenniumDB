#include "path_validator.h"

#include <iostream>

using namespace std;

PathValidator::PathValidator(OpPropertyPath& path) {
    //TODO: STRUCT para las transiciones
    //TODO: STRUCT para los nodos
    //TODO: Vector de nodos
    //TODO: Orden de Op
    //TODO: Mover instanciación de PathValidator fuera de OpMatch
    //TODO: Destructor de OpPropertyPath
    //TODO: A{m,n} => sequence(A...(m veces)...,A,A?,...(n veces)...,A?)

    total_states = 2;
    transitions.push_back( vector<tuple<uint32_t, string, bool>>() );
    transitions.push_back( vector<tuple<uint32_t, string, bool>>() );

    limits.push( {0, 1} );
    path.path->accept_visitor(*this);
    // generate_definitive_transitions();
}


void PathValidator::visit(OpPathAlternatives& path) {
    auto initial_limits = limits.top();
    limits.pop();
    for (auto& alternative : path.alternatives) {
        limits.push( initial_limits );
        alternative->accept_visitor(*this);
    }
}


void PathValidator::visit(OpPathSequence& path) {
    auto initial_limits = limits.top();
    limits.pop();

    transitions.push_back( vector<tuple<uint32_t, string, bool>>() );
    auto next_start = total_states;
    auto next_end = total_states++;
    limits.push({ std::get<0>(initial_limits), next_start});
    path.sequence[0]->accept_visitor((*this));

    for (size_t i = 1; i + 1 < path.sequence.size(); i++) {
        limits.push({next_start, total_states });
        transitions.push_back( vector<tuple<uint32_t, string, bool>>() );
        next_start = total_states;
        next_end = total_states++;
        path.sequence[i]->accept_visitor(*this);
    }
    limits.push({ next_end, std::get<1>(initial_limits) });
    path.sequence[path.sequence.size() - 1]->accept_visitor(*this);
}

/*
void PathValidator::visit(OpPathSuffix& path) {
    auto initial_limits = limits.top();
    auto start = std::get<0>(initial_limits);
    auto end   = std::get<1>(initial_limits);
    limits.pop();
    if (path.min == 0 && path.max == 1) {
        // ? suffix
        // TODO:
        //transitions[std::get<0>(new_limits)].push_back({std::get<1>(new_limits), "", false});
    }
    else if (path.min == 0 && path.max == OpPathSuffix::MAX) {
        //  suffix

        // Epsilon transitions
        /
        transitions[start].push_back({end, "", false});
        limits.push({ start, start });
        path.op_path->accept_visitor(*this);
        /
        // create 2 new states
        
        auto new_start = total_states++;
        auto new_end   = total_states++;
        transitions.push_back( vector<tuple<uint32_t, string, bool>>() );
        transitions.push_back( vector<tuple<uint32_t, string, bool>>() );

        // Epsilon transitions
        transitions[start].push_back({end, "", false});
        transitions[start].push_back({new_start, "", false});
        transitions[new_end].push_back({end, "", false});
        transitions[end].push_back({new_start, "", false});

        limits.push({ new_start, new_end });
        path.op_path->accept_visitor(*this);
    }
    else if (path.min == 1 && path.max == OpPathSuffix::MAX) {
        // + suffix
        // TODO:
        //transitions[std::get<1>(new_limits)].push_back({std::get<0>(new_limits), "", false});
    } else {
        // TODO: {min, max} suffix
    }
}
*/
void PathValidator::visit(OpPathKleeneStar&) { }
void PathValidator::visit(OpPathEpsilon&)    { }


void PathValidator::visit(OpPathAtom& path) {
    auto my_limits = limits.top();
    limits.pop();

    auto start = std::get<0>(my_limits);
    auto end   = std::get<1>(my_limits);

    transitions[start].push_back(std::make_tuple(end, path.atom, path.inverse));
}


void PathValidator::print() {
    for (size_t i = 0; i <  transitions.size(); i++) {
        for (auto& transition : transitions[i]) {
            cout << " " << i << "-["
                 << (std::get<2>(transition) ? "^" : "") << std::get<1>(transition)
                 << "]->" << std::get<0>(transition) << "\n";
        }
    }

    cout << "================\n";

    for (size_t i = 0; i < epsilon_closures.size(); i++) {
        cout << i << ": [ ";
        for (auto& state : epsilon_closures[i]) {
            cout << state << " ";
        }
        cout << "]\n";
    }

    cout << "================\n";

    for (size_t i = 0; i <  definitive_transitions.size(); i++) {
        for (auto& transition : definitive_transitions[i]) {
            cout << " " << i << "-[" << (std::get<2>(transition) ? "^" : "") << std::get<1>(transition) << "]->" << std::get<0>(transition) << "\n";
        }
    }
}

void PathValidator::generate_definitive_transitions() {
    // Calculate closures
    for (size_t state = 0; state < total_states; state++) {
        std::set<uint32_t> epsilon_closure;
        std::stack<uint32_t> stack;

        epsilon_closure.insert(state);
        stack.push(state);
        while (!stack.empty()) {
            auto current_state = stack.top();
            stack.pop();

            for (auto& transition : transitions[current_state]) {
                if (std::get<1>(transition).empty()) {
                    auto reached_state = std::get<0>(transition);
                    if (epsilon_closure.find(reached_state) == epsilon_closure.end()) {
                        epsilon_closure.insert(reached_state);
                        stack.push(reached_state);
                    }
                }
            }
        }
        epsilon_closures.push_back( std::move(epsilon_closure) );
    }

    // Generate transitions
    // TODO:
    definitive_transitions.resize(total_states);
    for (size_t state_from = 0; state_from < total_states; state_from++) {
        for (auto closure_from : epsilon_closures[state_from]) {
            for (auto& transition : transitions[closure_from]) {
                const uint32_t to_state  = std::get<0>(transition);
                const string type        = std::get<1>(transition);
                const bool inverted      = std::get<2>(transition);

                if (!type.empty()) {
                    for (auto closure_to : epsilon_closures[to_state]) {
                        // TODO: may add duplicateds?
                        definitive_transitions[state_from].push_back(
                            std::make_tuple(closure_to, type, inverted)
                        );
                    }
                    // from-*->closure_from-[type]->to-*->closure_to
                    // add from-[type]->closure_to
                }
            }
        }
    }
}


// UNUSED
void PathValidator::visit(OpSelect&) { }
void PathValidator::visit(OpMatch&) { }
void PathValidator::visit(OpFilter&) { }
void PathValidator::visit(OpConnection&) { }
void PathValidator::visit(OpConnectionType&) { }
void PathValidator::visit(OpLabel&) { }
void PathValidator::visit(OpOptional&) { }
void PathValidator::visit(OpProperty&) { }
void PathValidator::visit(OpTransitiveClosure&) { }
void PathValidator::visit(OpGraphPatternRoot&) { }
void PathValidator::visit(OpOrderBy&) { }
void PathValidator::visit(OpGroupBy&) { }
void PathValidator::visit(OpUnjointObject&) { }

void PathValidator::visit(OpPropertyPath&) { }
void PathValidator::visit(OpPath&) { }
