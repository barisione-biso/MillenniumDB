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
    std::vector<std::vector<Transition>> connections;
    std::vector<uint32_t> incidence_vector;
    uint32_t total_states = 1;

    PathAutomaton();
    ~PathAutomaton() = default;

    void print();

    // Add states from other to this, rename 'other' states, update 'other'
    // end to be consistent with rename. Don't update 'other' connections
    void rename_and_merge(PathAutomaton& other);

    void connect(Transition transition);

    // Add a transition (from, to, "", false)
    void add_epsilon_transition(uint32_t from, uint32_t to);

    void optimize_automata();
    private:
        // Método 1
        void merge_empty_states();
        void delete_epsilon_from(Transition transition);

        // TODO: Comenar porque no incluye al mismo estado en la clausura
        std::set<uint32_t> get_epsilon_closure(uint32_t state);
        // Remove epsilon transition and unreachable states
        void clean_automata(uint32_t state);


        void delete_unreachable_states();
};

#endif // BASE__PATH_AUTOMATON_H_
