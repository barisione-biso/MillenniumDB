#ifndef GRAMMAR__VISITORS__ASSIGN_CONNECTIONS_H
#define GRAMMAR__VISITORS__ASSIGN_CONNECTIONS_H

#include "grammar/ast.h"
#include "grammar/exceptions.h"

#include <map>

#include <boost/variant.hpp>

#ifndef GRAMMAR__VISITORS
    typedef std::vector<std::array<uint_fast32_t, 3>> connect_vect;
    typedef std::map<std::string, uint_fast32_t> str_int_map;
#endif

namespace visitors {
    // Fifth visitor retrieves the connections between
    // nodes with a corresponding edge. Returns an array
    // of 3 VarIDs: (1, 2, 3)  <-> (1) -[2]-> (3) .
    class assignConnections
        : public boost::static_visitor< void >
    {
        str_int_map idMap;
        connect_vect connections;

        public:

        // Constructor 
        assignConnections(str_int_map & idMap)
            : idMap(idMap) {}
        
        connect_vect get_connections() {
            return connections;
        }
        
        void operator()(ast::root const& r) {
            for(auto const& lPattern: r.graphPattern_) {
                (*this)(lPattern);
            }
        }

        void operator()(ast::linear_pattern const& lPattern) {
            uint_fast32_t id1, id2, id3;
            id1 = idMap.at(lPattern.root_.variable_);
            for(auto &sPath: lPattern.path_) {
                id2 = idMap.at(sPath.edge_.variable_);
                id3 = idMap.at(sPath.node_.variable_);
                if(sPath.edge_.isright_)
                    connections.push_back({id1, id2, id3});
                else
                    connections.push_back({id3, id2, id1});
                id1 = id3;
            }
        }
        
    }; // class assignConnections

}; // namespace visitors

#endif // GRAMMAR__VISITORS__ASSIGN_CONNECTIONS_H