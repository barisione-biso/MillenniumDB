#include "path_automaton.h"

#include <iostream>

using namespace std;

uint32_t PathAutomaton::id = 0;

PathAutomaton::PathAutomaton() {
    start = new State(PathAutomaton::id);
    PathAutomaton::id++;
    end = new State(PathAutomaton::id);
    PathAutomaton::id++;
}

void PathAutomaton::print() {
     for (auto i = conections.begin(); i != conections.end(); ++i) {
        for (auto& j : i->second) {
            cout << i->first << "=[" << j.second << "]=>" << j.first->get_id() << "\n";
        }
    }

}


void PathAutomaton::merge_with_automaton(PathAutomaton automaton) {
    //conections.insert(automaton.conections.begin(), automaton.conections.end());
    conections.merge(automaton.conections);
}

void PathAutomaton::connect_states(State* from, State* to, std::string type) {
    if (conections.find(from->get_id()) == conections.end()) {
        std::vector<pair<State*, std::string>> new_vec;
        new_vec.push_back(make_pair(to, type));
        conections[from->get_id()] = new_vec;
    } else {
        conections[from->get_id()].push_back(make_pair(to, type));
    }
}


State::State(uint32_t _id) :
    id   (_id)  { }

uint32_t State::get_id() {
    return id;
}
