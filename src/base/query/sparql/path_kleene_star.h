#pragma once

#include <memory>
#include <vector>

#include "base/query/sparql/path.h"
#include "base/query/sparql/path_denull.h"

namespace SPARQL {

class PathKleeneStar : public IPath {
public:
    std::unique_ptr<IPath> path;

    PathKleeneStar(std::unique_ptr<IPath> path) : path(PathDenull::denull(std::move(path))) { }

    PathKleeneStar(const PathKleeneStar& other) : path(other.path->duplicate()) { }

    PathType type() const override {
        return PathType::PATH_KLEENE_STAR;
    }

    std::unique_ptr<IPath> duplicate() const override {
        return std::make_unique<PathKleeneStar>(*this);
    }

    std::string to_string() const override {
        return "(" + path->to_string() + ")*";
    }

    std::unique_ptr<IPath> invert() const override {
        return std::make_unique<PathKleeneStar>(path->invert());
    }

    bool nullable() const override {
        return true;
    }

    RPQAutomaton get_rpq_base_automaton() const override {
        auto path_automaton = path->get_rpq_base_automaton();
        // Heuristic for kleen star construction
        if (path_automaton.get_total_states() == 2) {
            // Automaton with 2 states have only one connection from 0 to 1
            auto  new_automaton = RPQAutomaton();
            auto& transition    = path_automaton.from_to_connections[0][0];
            new_automaton.add_transition(Transition(0, 0, transition.type, transition.inverse));
            new_automaton.end_states.insert(new_automaton.get_start());
            return new_automaton;
        } else {
            // Connects all end states to start state
            for (const auto& end_state : path_automaton.end_states) {
                path_automaton.add_epsilon_transition(end_state, path_automaton.get_start());
            }
            // Makes start state final
            path_automaton.end_states.insert(path_automaton.get_start());
            return path_automaton;
        }
    }
};
} // namespace SPARQL