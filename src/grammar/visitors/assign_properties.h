#ifndef GRAMMAR__VISITORS__ASSIGN_PROPERTIES_H
#define GRAMMAR__VISITORS__ASSIGN_PROPERTIES_H

#include "grammar/ast.h"
#include "grammar/exceptions.h"

#include <map>

#include <boost/variant.hpp>

#ifndef GRAMMAR__VISITORS
    typedef std::map<std::string, std::map<std::string, uint_fast32_t>> str_str_int_map;
    typedef std::map<uint_fast32_t, std::map<std::string, ast::value>> int_str_value_map;
    typedef std::map<std::string, uint_fast32_t> str_int_map;
#endif

namespace visitors {
    // Auxiliary visitor to get the correspoding index to boost::variant
    // according to value
    class whichVisitor
        : public boost::static_visitor<int> {
    public:
        int operator()(ast::value const& val) const {return val.which();}
        int operator()(ast::element const&) const {return -1;}
    }; // class whichVisitor

    // Fourth visitor retrieves the properties asociated 
    // to each VarID and checks consistency in the type
    // of values at WHERE and MATCH statements. Returns
    // a VarID to map of Key Values map. 
    class asssignProperties
        : public boost::static_visitor<void>
    {
        str_int_map idMap;
        int_str_value_map propertyMap;
        str_str_int_map seenStatementValueType;

        public:

        // Constructor 
        asssignProperties(str_int_map & idMap)
            : idMap(idMap) { }

        int_str_value_map get_propertyMap() {
            return propertyMap;
        }
        
        void operator()(ast::root const& r) {
            for(auto const& lPattern: r.graphPattern_) {
                (*this)(lPattern);
            }
            (*this)(r.where_);
            boost::apply_visitor(*this, r.selection_);
        }

        void operator()(ast::linear_pattern const& lPattern) {
            (*this)(lPattern.root_);
            for(auto &sPath: lPattern.path_) {
                (*this)(sPath.edge_);
                (*this)(sPath.node_);
            }
        }

        void operator()(ast::edge const& edge) {
            for(auto & prop: edge.properties_) {
                propertyMap[idMap.at(edge.variable_)][prop.key_] =  prop.value_;
            }   
        }

        void operator()(ast::node const& node) {
            for(auto & prop: node.properties_) {
                propertyMap[idMap.at(node.variable_)][prop.key_] = prop.value_;
            }
        }

        // Wrapper for the case of the optional where statement in ast::root 
        void operator()(boost::optional<ast::formula> const& formula) 
        {
            if(formula) {
                ast::formula realFormula = static_cast<ast::formula>(formula.get());
                (*this)(realFormula);
            }
        }

        void operator()(ast::formula const& formula) {
            boost::apply_visitor(*this, formula.root_.content_);
            for (auto & sFormula: formula.path_) {
                boost::apply_visitor(*this, sFormula.cond_.content_);
            }
        }

        

        void operator()(ast::statement const& stat) {

            // Check consistencies
            int storedWhich = boost::apply_visitor(whichVisitor(), stat.rhs_);
            if(storedWhich >= 0) { // Check type consistency against declared properties
                uint_fast32_t id_ = idMap.at(stat.lhs_.variable_);
                auto entMap = propertyMap.find(id_);
                if (entMap != propertyMap.end()) {
                    auto foundIt = entMap->second.find(stat.lhs_.key_);
                    if(foundIt != entMap->second.end()) {
                        if (storedWhich != foundIt->second.which()) {
                            throw ast::TypeError(stat.lhs_.variable_);
                        }
                    }
                }
                else { // Check type consistency against other statements
                    auto foundMap = seenStatementValueType.find(stat.lhs_.variable_);
                    if (foundMap != seenStatementValueType.end()) {
                        auto foundIt = foundMap->second.find(stat.lhs_.key_);
                        if (foundIt != foundMap->second.end()) {
                            if (storedWhich != (int)foundIt->second) {
                                throw ast::TypeError(stat.lhs_.variable_);
                            }
                        }
                        else {
                            foundMap->second[stat.lhs_.key_] = storedWhich;
                        }
                    } 
                    else {
                        seenStatementValueType[stat.lhs_.variable_][stat.lhs_.key_] = storedWhich;
                    }
                }
            } 
        }


        void operator()(std::vector<ast::element> const& container) {
            uint_fast32_t varID;
            int auxVarID;
            for(auto const& elem: container) {
                varID = idMap.at(elem.variable_);
                auto foundVar = propertyMap.find(varID);
                if (foundVar != propertyMap.end()) {
                    auto foundKey = foundVar->second.find(elem.key_); 
                        if(foundKey != foundVar->second.end())
                            continue;
                }
                auxVarID = idMap.at(elem.variable_ + "." + elem.key_);
                propertyMap[varID][elem.key_] = auxVarID;
            }   
        }
            
        void operator()(ast::all_ const&) {}

        
        
    }; // class assignProperties

    

}; // namespace visitors

#endif // GRAMMAR__VISITORS__ASSIGN_PROPERTIES_H