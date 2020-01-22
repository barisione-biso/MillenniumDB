#ifndef GRAMMAR__VISITORS__ASSIGN_ENTITIES_H
#define GRAMMAR__VISITORS__ASSIGN_ENTITIES_H

#include "grammar/ast.h"
#include "grammar/exceptions.h"
#include "base/graph/graph_element.h"

#include <map>

#include <boost/variant.hpp>

#ifndef GRAMMAR__VISITORS
    typedef std::map<std::string, uint_fast32_t> str_int_map;
#endif

namespace visitors {

    // Second visitor assigns Entities to VarIDs
    // and checks for inconsistent entities present
    // at MATCH statement. Outputs a VarID to Entity map.
    class assignEntities
        : public boost::static_visitor<void>
    {
    private:
        str_int_map idMap;
        std::map<uint_fast32_t, ElementType> id2type;

    public:
        assignEntities(str_int_map &idMap)
            : idMap(idMap) {}

        std::map<uint_fast32_t, ElementType> get_id2type() {
            return id2type;
        }

        void operator()(ast::root &r) {
            for (auto &lPattern: r.graphPattern_) {
                (*this)(lPattern);
            }
            boost::apply_visitor(*this, r.selection_);
        }

        void operator()(ast::edge &edge) {
            auto search = id2type.find(idMap.at(edge.variable_));
            if (search != id2type.end()) {
                if (search->second != ElementType::EDGE)
                    throw ast::EntityError(edge.variable_);
            }
            id2type[idMap.at(edge.variable_)] = ElementType::EDGE;
        }

        void operator()(ast::node & node) {
            auto search = id2type.find(idMap.at(node.variable_));
            if (search != id2type.end()) { // Found
                if (search->second != ElementType::NODE)
                    throw ast::EntityError(node.variable_);
            }
            id2type[idMap.at(node.variable_)] = ElementType::NODE;
        }

        void operator()(ast::linear_pattern & lPattern) {
            (*this)(lPattern.root_);
            for(auto &sPath: lPattern.path_) {
                (*this)(sPath.edge_);
                (*this)(sPath.node_);
            }
        }

        void operator()(std::vector<ast::element> & container) {
            for (auto &elem: container) {
                (*this)(elem);
            }
        }

        void operator()(ast::element &) {
            // TODO: Check variable is present in match
        }

        // Dummy leaves
        // void operator()(ast::value & val)  {
        //     boost::apply_visitor(*this, val);
        // }
        void operator() (ast::all_ &) {}
        // void operator() (std::string & text)  {}
        // void operator() (int & n)  {}
        // void operator() (float & n)  {}
        // void operator() (bool const& b) const {}

    }; // class assignEntities

}; // namespace visitors

#endif // GRAMMAR__VISITORS__ASSIGN_ENTITIES_H