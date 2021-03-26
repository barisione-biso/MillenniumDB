#ifndef BASE__PATH_AUTOMATON_H_
#define BASE__PATH_AUTOMATON_H_

#include <string>
#include <map>
#include <vector>

class State {
    public:
        State(uint32_t id);
        ~State()=default;
        uint32_t get_id();
        void print();

    private:
        uint32_t id;
};


class PathAutomaton {
public:
    PathAutomaton();
    ~PathAutomaton() = default;

    void print();
    void merge_with_automaton(PathAutomaton automaton);
    void connect_states(State* from, State* to, std::string type);
    State* start;
    State* end;
    std::map<uint32_t, std::vector<std::pair<State*, std::string>>> conections;


private:
    static uint32_t id;
};

#endif // BASE__PATH_AUTOMATON_H_
