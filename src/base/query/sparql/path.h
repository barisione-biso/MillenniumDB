#pragma once

#include <memory>

#include "base/query/sparql/iri.h"
#include "parser/query/paths/automaton/rpq_automaton.h"

namespace SPARQL {

enum class PathType
{
    PATH_ALTERNATIVES,
    PATH_SEQUENCE,
    PATH_ATOM,
    PATH_KLEENE_STAR,
    PATH_OPTIONAL,
};

class IPath {
public:
    virtual ~IPath() = default;

    virtual PathType type() const = 0;

    virtual std::unique_ptr<IPath> duplicate() const = 0;

    virtual std::string to_string() const = 0;

    // true if the path can be empty
    virtual bool nullable() const = 0;

    RPQAutomaton get_rpq_automaton(std::function<ObjectId(const std::string&)> f) const {
        auto automaton = get_rpq_base_automaton();
        automaton.transform_automaton(f);
        return automaton;
    }

    virtual RPQAutomaton get_rpq_base_automaton() const = 0;
};
} // namespace SPARQL
