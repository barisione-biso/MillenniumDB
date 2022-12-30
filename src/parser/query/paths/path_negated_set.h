#pragma once

#include <memory>
#include <vector>

#include "parser/query/paths/path.h"
#include "parser/query/paths/path_atom.h"

class PathNegatedSet : public IPath {
public:
    std::vector<PathAtom> negated_set;

    PathNegatedSet(std::vector<PathAtom> _negated_set) :
        negated_set(std::move(_negated_set)) { }

    PathNegatedSet(const PathNegatedSet& other) {
        for (const auto& atom : other.negated_set) {
            negated_set.push_back(atom);
        }
    }

    PathType type() const override {
        return PathType::PATH_NEGATED_SET;
    }

    std::unique_ptr<IPath> duplicate() const override {
        return std::make_unique<PathNegatedSet>(*this);
    }

    std::string to_string() const override {
        std::string result = "!(";
        for (const auto& atom : negated_set) {
            result += atom.to_string() + "|";
        }
        result.pop_back();
        result += ")";
        return result;
    }

    std::ostream& print_to_ostream(std::ostream& os, int indent = 0) const override {
        os << std::string(indent, ' ');
        os << "PathNegatedSet(";
        for (const auto& atom : negated_set) {
            os << atom.atom << "|";
        }
        os << ")\n";
        return os;
    }

    std::unique_ptr<IPath> invert() const override {
        // TODO: implement this
        throw std::runtime_error("PathNegatedSet::invert() not implemented");
    }

    bool nullable() const override {
        return false;
    }

    RPQAutomaton get_rpq_base_automaton() const override {
        // TODO: implement this
        throw std::runtime_error("PathNegatedSet::get_rpq_base_automaton() not implemented");
    }

    RDPQAutomaton get_rdpq_base_automaton() const override {
        // TODO: implement this
        throw std::runtime_error("PathNegatedSet::get_rpq_base_automaton() not implemented");
    }
};
