#ifndef BASE__PATH_AUTOMATON_H_
#define BASE__PATH_AUTOMATON_H_

#include <string>
#include <set>
#include <vector>

#include "base/ids/object_id.h"

/*
A property path is represented by a regular expression
in a query. The following classes are used to build
an automaton that accepts this regular expression.
*/

struct Transition {
    // Transition object represents a transition of the automaton.

    uint32_t from; // Automaton start state
    uint32_t to;   // Automaton end state

    // Label of the transition. Epsilon transitions are represents by a empty label
    std::string label;

    // Inverse represents if the direction is from-to(inverse=false) or
    // to-from(inverse=true)
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
    /*
    Represents a transition of the automaton like Transition class,
    but label is represented by a ObjectId instead of the string.
    This objects are used by binding_id_iter operators.
    */
    uint32_t to;
    ObjectId label;
    bool inverse;

    TransitionId(uint32_t to, ObjectId label, bool inverse) :
        to        (to),
        label     (label),
        inverse   (inverse) { }
};


/*
PathAutomaton represents a No Deterministic Finite Automaton
with epsilon transitions. This class builds an automaton, and transform
it into an automaton without epsilon transitions.

The automaton is built using Thompson Algorithm, each operator allowed by the
language is represented in the corresponding OpPath subclass.

There are some methods used to reduce the automaton, but it's
no guaranteed that the automaton will be optimal or deterministic.
TODO: Explain better final state.
TODO: Explain automaton transformations
The final
automaton will have at maximum two final states, the start state and a auxiliary
state that is added to be final state.

States of automaton are not emulated by a specific class. A state is only
represented by a number i, that indicates that the transitions of this
state are stored in the i position of the from_to_connections, to_from_connections
and transitions vectors.
*/
class PathAutomaton {
private:

    // Transitions that starts from a state (stored in i-position).
    // This vector will be set in property path plan, before build

    // Start state, always is 0
    uint32_t start = 0;

    // Final state will be set and the end of automaton transformation
    uint32_t final_state;

    // Number of states
    uint32_t total_states = 1;

    // Check if two states are mergeable and merge them if is posible.
    void delete_mergeable_states();

    // Return  epsilon closure of state
    // state is not included to avoid redundant iteration
    std::set<uint32_t> get_epsilon_closure(uint32_t state);

    // Delete states that can not be reached from start
    void delete_unreachable_states();

    // Delete states that can not reach to any state of end_states set
    void delete_absortion_states();

    // Return a set with reachable states from start.
    std::set<uint32_t> get_reachable_states_from_start();

    // Return a set with reachable states from a state in end set.
    std::set<uint32_t> get_reachable_states_from_end();

    // Connections that starts or reachs to source will be start or reach to destiny
    void merge_states(uint32_t destiny, uint32_t source);

    // Compute the minimum distance between final_state and a state of the automaton
    void calculate_distance_to_final_state();

    // Add a extra state which is final, end set will be cleared. If start is in end,
    // start_is_final will be true
    void set_final_state();

    // Delete epsilon transitions of the automaton
    void delete_epsilon_transitions();

public:

    // Transitions that starts from a i-state (stored in i-position).
    std::vector<std::vector<Transition>> from_to_connections;

    // Transitions that reaches to a state (stored in i-position).
    std::vector<std::vector<Transition>> to_from_connections;

    // the binding id iter operator
    std::vector<std::vector<TransitionId>> transitions;

    // End states before of the transformation to automaton transformation
    std::set<uint32_t> end_states;

    // True if the start state is final
    bool start_is_final = false;

    // Stores the distance to end state. It can be used by
    // AStar algorithm in enum and check binding_id_iter algorithms.
    std::vector<uint32_t> distance_to_final;


    PathAutomaton()  = default;
    ~PathAutomaton() = default;

    // Access  and modify attibute methods
    inline std::vector<TransitionId> get_state_transitions(size_t state) const noexcept{ return transitions[state]; }
    inline uint32_t get_start() const noexcept { return start; }
    inline uint32_t get_total_states() const noexcept  { return total_states; }
    inline uint32_t get_final_state() const noexcept  { return final_state; }

    void print();

    // Add states from other to this, rename 'other' states, update 'other'
    // end states to be consistent with rename. Don't update 'other' connections
    void rename_and_merge(PathAutomaton& other);

    // Add a transition to automaton
    void connect(Transition transition);

    // Add a transition (from, to, "", false)
    void add_epsilon_transition(uint32_t from, uint32_t to);

    // Delete the epsilon transitions, and apply some methods to reduce
    // automaton size. Optimal or final deterministic automaton is not guaranteed
    void transform_automaton();



};

#endif // BASE__PATH_AUTOMATON_H_
