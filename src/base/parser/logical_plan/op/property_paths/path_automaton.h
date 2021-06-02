#ifndef BASE__PATH_AUTOMATON_H_
#define BASE__PATH_AUTOMATON_H_

#include <string>
#include <set>
#include <tuple>
#include <vector>

#include "base/ids/object_id.h"

/*
TODO: Explain classes and delete unnecesary includes
*/

struct Transition {
    uint32_t from;
    uint32_t to;
    std::string label;
    bool inverse;

    Transition(uint32_t from, uint32_t to, std::string label, bool inverse) :
        from      (from),
        to        (to),
        label     (label),
        inverse   (inverse) { }

    bool operator==(Transition other) {
        return from == other.from && to == other.to && label == other.label;
    }
};

struct TransitionId {
    // uint32_t from;
    uint32_t to;
    ObjectId label;
    bool inverse;

    TransitionId(uint32_t to, ObjectId label, bool inverse) :
        to        (to),
        label     (label),
        inverse   (inverse) { }
};



class PathAutomaton {
    /*
    TODO: Explain how automaton works. Explain when empty structs will be filled
    */

public:
    std::vector<std::vector<Transition>> from_to_connections;
    std::vector<std::vector<Transition>> to_from_connections;
    std::vector<uint32_t> distance_to_final;

    std::set<uint32_t> end;
    uint32_t start = 0;
    uint32_t final_state;
    std::vector<std::vector<TransitionId>> transitions;
    uint32_t total_states = 1;
    bool start_is_final = false;

    PathAutomaton()  = default;
    ~PathAutomaton() = default;

    void print();

    // Add states from other to this, rename 'other' states, update 'other'
    // end states to be consistent with rename. Don't update 'other' connections
    void rename_and_merge(PathAutomaton& other);

    void connect(Transition transition);

    // Add a transition (from, to, "", false)
    void add_epsilon_transition(uint32_t from, uint32_t to);

    // Delete extra nodes and transitions
    void optimize_automata();

    // Add a extra state which is final, end set will be cleared. If start is in end,
    // start_is_final will be true
    void set_final_state();

    // Compute the minimum distance between final_state and a state of the automaton
    void calculate_distance_to_final_state();

    inline void add_end_state(uint32_t state) { end.insert(state); }

private:
    // Check if two states are mergeable and do the merge.
    void delete_mergeable_states();

    // set returned  of ep. closure of 's' doesn't include s for avoid
    // redundant iteration during optimization.
    std::set<uint32_t> get_epsilon_closure(uint32_t state);

    // Delete states that can not be reached from start
    void delete_unreachable_states();

    // Absortion states cannot reach a any final state
    void delete_absortion_states();

    // Return a set with reachable states from start.
    std::set<uint32_t> get_reachable_states_from_start();

    // Return a set with reachable states from a state in end set.
    std::set<uint32_t> get_reachable_states_from_end();

    // Connections from source or to source will be from or to destiny.
    // Be careful, all source transitions will be removed
    void merge_states(uint32_t destiny, uint32_t source);

};

#endif // BASE__PATH_AUTOMATON_H_
