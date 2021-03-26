#ifndef BASE__PATH_AUTOMATON_H_
#define BASE__PATH_AUTOMATON_H_

#include <string>
#include <map>
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
};

class PathAutomaton {
public:
    uint32_t start = 0;
    std::set<uint32_t> end;
    std::vector<std::vector<Transition>> connections;
    uint32_t total_states = 1;

    PathAutomaton();
    ~PathAutomaton() = default;

    void print();

    // Add states from other to this, rename 'other' states, update 'other' end to be consistent with rename. Don't update 'other' connections
    void merge_with_automaton(PathAutomaton& other);

    void connect(Transition transition);

    // Add a transition (from, to, "", false)
    void add_epsilon_transition(uint32_t from, uint32_t to);
};

#endif // BASE__PATH_AUTOMATON_H_
