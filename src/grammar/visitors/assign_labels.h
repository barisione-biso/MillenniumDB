#ifndef GRAMMAR__VISITORS__ASSIGN_LABELS_H
#define GRAMMAR__VISITORS__ASSIGN_LABELS_H

#include "grammar/ast.h"
#include "grammar/exceptions.h"

#include <map>

#include <boost/variant.hpp>

#ifndef GRAMMAR__VISITORS
    typedef std::map<uint_fast32_t, std::vector<std::string>> int_strs_map;
    typedef std::map<std::string, uint_fast32_t> str_int_map;
#endif

namespace visitors {
    // Third visitor retrieves the labels asociated to
    // each VarID where it corresponds. Returns a VarID (unsigned) 
    // to vector of Labels (str) map.
    class assignLabels
        : public boost::static_visitor<void>
    {
        str_int_map idMap;
        int_strs_map labelMap;

        public:

        // Constructor 
        assignLabels(str_int_map & idMap)
            : idMap(idMap) {}

        int_strs_map get_labelMap() {
            return labelMap;
        }
        
        void operator()(ast::root const& r) {
            for(auto const& lPattern: r.graphPattern_) {
                (*this)(lPattern);
            }
            ;
        }

        void operator()(ast::linear_pattern const& lPattern) {
            (*this)(lPattern.root_);
            for(auto &sPath: lPattern.path_) {
                (*this)(sPath.edge_);
                (*this)(sPath.node_);
            }

            ;
        }

        void operator()(ast::edge const& edge) {
            uint_fast32_t varID = idMap.at(edge.variable_);
            auto found = labelMap.find(varID);
            if(found != labelMap.end())
                found->second.insert(found->second.end(), edge.labels_.begin(), edge.labels_.end());
            else 
                labelMap.emplace(std::make_pair(varID, std::vector<std::string>(edge.labels_)));

            ;
        }

        void operator()(ast::node const& node) {
            uint_fast32_t varID = idMap.at(node.variable_);
            auto found = labelMap.find(varID);
            if(found != labelMap.end())
                found->second.insert(found->second.end(), node.labels_.begin(), node.labels_.end());
            else 
                labelMap.emplace(std::make_pair(varID, std::vector<std::string>(node.labels_)));

            ;
        }

    }; // class assignLabels

}; // namespace visitors

#endif // GRAMMAR__VISITORS__ASSIGN_LABELS_H