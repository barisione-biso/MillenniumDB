#ifndef BASE__PATH_AUTOMATON_H_
#define BASE__PATH_AUTOMATON_H_

#include <string>
#include <map>
#include <tuple>
#include <vector>


class PathAutomaton {
public:
    uint32_t start;
    uint32_t end;
    std::map<uint32_t, std::vector<std::tuple<uint32_t, std::string, bool>>> conections;

    PathAutomaton();
    ~PathAutomaton() = default;

    void print();
    void merge_with_automaton(PathAutomaton automaton);
    void connect_states(uint32_t from, uint32_t to, std::string type, bool inverse);

private:
    static uint32_t id;
};

#endif // BASE__PATH_AUTOMATON_H_
