#pragma once

#include <cassert>
#include <memory>
#include <string>

#include "base/query/sparql/sparql_element.h"
#include "parser/query/op/op.h"

enum class PathSemantic
{
    ANY_SHORTEST,
    ALL_SHORTEST,
};

namespace SPARQL {

class OpPath : public Op {
public:
    Var           var;
    SparqlElement subject;
    SparqlElement object;
    PathSemantic  semantic;

    std::unique_ptr<IPath> path;

    OpPath(Var                    _var,
           SparqlElement          _subject,
           SparqlElement          _object,
           PathSemantic           _semantic,
           std::unique_ptr<IPath> _path) :
        var      (_var),
        subject  (std::move(_subject)),
        object   (std::move(_object)),
        semantic (_semantic),
        path     (std::move(_path)) {
            assert(path != nullptr);
        }

    void accept_visitor(OpVisitor& visitor) override {
        visitor.visit(*this);
    }

    std::set<Var> get_vars() const override {
        std::set<Var> res;
        if (subject.is_var()) {
            res.insert(subject.to_var());
        }
        if (object.is_var()) {
            res.insert(object.to_var());
        }
        res.insert(var);
        return res;
    }

    std::ostream& print_to_ostream(std::ostream& os, int indent = 0) const override {
        os << std::string(indent, ' ') << "OpPath(" << (semantic == PathSemantic::ANY_SHORTEST ? "ANY" : "ALL") << " SHORTEST, " << var
           << ", " << subject << ", " << object << ", " << path->to_string() << ")\n";
        return os;
    }
};
} // namespace SPARQL