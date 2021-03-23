#ifndef BASE__PATH_VALIDATOR_H_
#define BASE__PATH_VALIDATOR_H_

#include <string>
#include <stack>
#include <tuple>
#include <vector>


#include "base/parser/logical_plan/op/op_path.h"
#include "base/parser/logical_plan/op/op_property_path.h"
#include "base/parser/logical_plan/op/op_path_alternatives.h"
#include "base/parser/logical_plan/op/op_path_sequence.h"
#include "base/parser/logical_plan/op/op_path_suffix.h"
#include "base/parser/logical_plan/op/op_path_atom.h"
#include "base/parser/logical_plan/op/op_path_kleene_star.h"
#include "base/parser/logical_plan/op/op_path_epsilon.h"

/*

PathValidator works like a AFND with empty transictions visit using Thomson Algorithm

*/

class PathValidator : OpVisitor {
public:
    PathValidator(OpPropertyPath& path);
    ~PathValidator() = default;
    void print();

private:
    std::vector<
       std::vector<std::tuple<uint32_t, std::string, bool>>
    > transitions;
    std::stack<std::tuple<uint32_t, uint32_t>> limits;
    uint32_t total_states;

    std::vector<
       std::vector<std::tuple<uint32_t, std::string, bool>>
    > definitive_transitions;
    std::vector<std::set<uint32_t>> epsilon_closures;

    // NFA constructors
    void visit(OpPathAlternatives&);
    void visit(OpPathSequence&);
    void visit(OpPathSuffix&);
    void visit(OpPathAtom&);
    void visit(OpPathKleenStar&);
    void visit(OpPathEpsilon&);

    void generate_definitive_transitions();

    // UNUSED
    void visit(OpSelect&);
    void visit(OpMatch&);
    void visit(OpFilter&);
    void visit(OpConnection&);
    void visit(OpConnectionType&);
    void visit(OpLabel&);
    void visit(OpOptional&);
    void visit(OpProperty&);
    void visit(OpTransitiveClosure&);
    void visit(OpGraphPatternRoot&);
    void visit(OpOrderBy&);
    void visit(OpGroupBy&);
    void visit(OpUnjointObject&);
    void visit(OpPropertyPath&);
    void visit(OpPath&);

};

#endif // BASE__PATH_VALIDATOR_H_
