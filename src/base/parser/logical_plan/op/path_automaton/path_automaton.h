#ifndef BASE__PATH_AUTOMATON_H_
#define BASE__PATH_AUTOMATON_H_

#include <string>
#include <set>
#include <tuple>
#include <vector>

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

class PathAutomaton {
public:
    uint32_t start = 0;
    std::set<uint32_t> end;
    std::vector<std::vector<Transition>> from_to_connections;
    std::vector<std::vector<Transition>> to_from_connections;
    uint32_t total_states = 1;

    PathAutomaton();
    ~PathAutomaton() = default;

    void print();

    // Add states from other to this, rename 'other' states, update 'other'
    // end states to be consistent with rename. Don't update 'other' connections
    void rename_and_merge(PathAutomaton& other);

    void connect(Transition transition);

    // Add a transition (from, to, "", false)
    void add_epsilon_transition(uint32_t from, uint32_t to);

    void optimize_automata();

    private:
        // Check if two states are mergeable and do the merge.
        void delete_mergeable_states();

        // set returned  of ep. closure of 's' doesn't include s for avoid
        // redundant iteration during optimization. Uses DFS
        std::set<uint32_t> get_epsilon_closure(uint32_t state);

        // Delete states that can not be reached from start
        void delete_unreachable_states();


        void delete_absortion_states();

        // Return a set with reachable states from start. Explores automaton with DFS
        std::set<uint32_t> get_reachable_states_from_start();

        // Return a set with reachable states from a state in end set. Use DFS
        std::set<uint32_t> get_reachable_states_from_end();

        // Connections from source or to source will be from or to destiny.
        // Be careful, all source transitions will be removed
        void merge_states(uint32_t destiny, uint32_t source);

};

#endif // BASE__PATH_AUTOMATON_H_
