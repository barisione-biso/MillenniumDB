#ifndef VISITOR_H
#define VISITOR_H

#include "ast.h"
#include "exceptions.h"

#include <boost/variant.hpp>
#include <boost/optional.hpp>

#include <map>
#include <set>
#include <exception>

unsigned const tabsize = 2;

// Typedefs for returning structs
typedef std::map<std::string, unsigned> StrIntMap;
typedef std::map<unsigned, std::vector<std::string>> IntVectStrMap;
typedef std::map<unsigned, std::map<std::string, ast::value>> IntStrValMap;
typedef std::map<std::string, std::map<std::string, unsigned>> StrStrIntMap;
typedef std::vector<std::array<unsigned, 3>> connectVect;
typedef std::map<unsigned, unsigned> IntEntMap;


enum Entity {
    NODE, 
    EDGE, 
    LABEL, 
    KEY, 
    VALSTRING, 
    VALINT, 
    VALFLOAT, 
    VALBOOL
};

namespace visitors {

    class whichVisitor
        : public boost::static_visitor<int>
    {
        public:

        int operator()(ast::value const& val) const {
            return val.which();
        }

        int operator()(ast::element const& elem) const {return -1;}
    }; // class whichVisitor


    class printer
        : public boost::static_visitor<>
    {
        unsigned indent;
        std::ostream& out; 

        public:

        // Constructor 
        printer(std::ostream& out, unsigned indent = 0)
            : out(out), indent(indent) {}
        
        void operator()(ast::root const& r) const {
            out << '{' << '\n';
            tab(indent+tabsize);
            out << "<Select> = ";
            boost::apply_visitor(printer(out, indent+tabsize), r.selection_);
            out << ',' << '\n';
            tab(indent+tabsize);
            out << "<GraphPattern> = [" << '\n';
            for (auto const& lPattern: r.graphPattern_) {
                tab(indent+2*tabsize);
                out << "<LinearPattern> = ";
                printer(out, indent+2*tabsize)(lPattern);
                out << ',' << '\n';
            }
            tab(indent+tabsize);
            out << ']' << '\n';
            tab(indent+tabsize);
            out << "<Where> = ";
            printer(out, indent+tabsize)(r.where_);
            out << '\n';
            tab(indent);
            out << '}' << '\n';
        }

        void operator()(boost::optional<ast::formula> const& formula) const {
            if(formula) {
                ast::formula realFormula = static_cast<ast::formula>(formula.get());
                out << '{' << '\n';
                tab(indent+tabsize);
                out << "<Formula> = ";
                boost::apply_visitor(printer(out, indent+tabsize), realFormula.root_);
                out << ',' << '\n';
                tab(indent+tabsize);
                out << "<Path> = [\n";
                for (auto const& sFormula: realFormula.path_) {
                    tab(indent+2*tabsize);
                    out << "<Op> = ";
                    boost::apply_visitor(printer(out, indent+2*tabsize), sFormula.op_);
                    out << ',' << '\n';
                    tab(indent+2*tabsize);
                    out << "<Formula> = ";
                    boost::apply_visitor(printer(out, indent+2*tabsize), sFormula.cond_);
                    out << ',' << '\n';
                }
                tab(indent+tabsize);
                out << ']' << '\n';
                tab(indent);
                out << '}';
            } else 
            {
                out << "[not present]" << '\n';
            }
        }

        void operator()(ast::formula const& formula) const {
            out << '{' << '\n';
            tab(indent+tabsize);
            out << "<Formula> = ";
            boost::apply_visitor(printer(out, indent+tabsize), formula.root_);
            out << ',' << '\n';
            tab(indent+tabsize);
            out << "<Path> = [\n";
            for (auto const& sFormula: formula.path_) {
                tab(indent+2*tabsize);
                out << "<Op> = ";
                boost::apply_visitor(printer(out, indent+2*tabsize), sFormula.op_);
                out << ',' << '\n';
                tab(indent+2*tabsize);
                out << "<Formula> = ";
                boost::apply_visitor(printer(out, indent+2*tabsize), sFormula.cond_);
                out << ',' << '\n';
            }
            tab(indent+tabsize);
            out << ']' << '\n';
            tab(indent);
            out << '}';

        }

        void operator()(ast::element const& elem) const {
            out << '{' << '\n';
            tab(indent+tabsize);
            out << "<Function> = ";
            printer(out, indent+tabsize)(elem.function_);
            out << ',' << '\n';
            tab(indent+tabsize);
            out << "<Variable> = ";
            printer(out, indent+tabsize)(elem.variable_);
            out << ',' << '\n';
            tab(indent+tabsize);
            out << "<Key> = ";
            printer(out, indent+tabsize)(elem.key_);
            out << '\n';
            tab(indent);
            out << '}'; 
        }

        void operator()(ast::statement const& stat) const {
            out << '{' << '\n';
            tab(indent+tabsize);
            out << "<LHS> = ";
            printer(out, indent+tabsize)(stat.lhs_);
            out << ',' << '\n';
            tab(indent+tabsize);
            out << "<Comparator> = ";
            boost::apply_visitor(printer(out, indent+tabsize), stat.comparator_);
            out << ',' << '\n';
            tab(indent+tabsize);
            out << "<RHS> = ";
            boost::apply_visitor(printer(out, indent+tabsize), stat.rhs_);
            out << '\n';
            tab(indent);
            out << '}'; 
        }

        void operator()(ast::value const& val) const {
            boost::apply_visitor(printer(out, indent), val);
        }

        void operator()(ast::edge const& edge) const {
            out << '{' << '\n';
            tab(indent+tabsize);
            out << "<Variable> = \"" << edge.variable_ << "\",\n";
            tab(indent+tabsize);
            out << "<Labels> = [" << '\n';
            for (auto const& label: edge.labels_) {
                tab(indent+2*tabsize);
                out << label << ",\n";
            }
            tab(indent+tabsize);
            out << "]," << '\n';
            tab(indent+tabsize);
            out << "<Direction> = ";
            if(edge.isright_)
                out << "->," << '\n';
            else
                out << "<-," << '\n';
            tab(indent+tabsize);
            out << "<Properties> = {" << '\n';
            for (auto const& prop: edge.properties_) {
                tab(indent+2*tabsize);
                out << '"' <<  prop.key_ << "\" : ";
                boost::apply_visitor(printer(out, indent+2*tabsize), prop.value_);
                out << ',' << '\n';
            }
            tab(indent+tabsize);
            out << '}' << '\n';
            tab(indent);
            out << '}';
        }

        void operator()(ast::all_ const& a) const {
            out << "<All>";
        }

        void operator()(ast::node const& node) const {
            out << '{' << '\n';
            tab(indent+tabsize);
            out << "<Variable> = \"" << node.variable_ << "\",\n";
            tab(indent+tabsize);
            out << "<Labels> = [" << '\n';
            for (auto const& label: node.labels_) {
                tab(indent+2*tabsize);
                out << '"' << label << '"' << ",\n";
            }
            tab(indent+tabsize);
            out << "]," << '\n';
            tab(indent+tabsize);
            out << "<Properties> = {" << '\n';
            for (auto const& prop: node.properties_) {
                tab(indent+2*tabsize);
                out << '"' <<  prop.key_ << "\" : ";
                boost::apply_visitor(printer(out, indent+2*tabsize), prop.value_);
                out << ',' << '\n';
            }
            tab(indent+tabsize);
            out << '}' << '\n';
            tab(indent);
            out << '}';
        }

        void operator()(ast::linear_pattern const& lPattern) const {
            out << '{' << '\n';
            tab(indent+tabsize);
            out << "<Root> = ";
            printer(out, indent+tabsize)(lPattern.root_);
            out << ',' << '\n';
            tab(indent+tabsize);
            out << "<Path> = [\n"; 
            for(auto const& stepPath: lPattern.path_) {
                tab(indent+2*tabsize);
                out << "<Edge> = ";
                printer(out, indent+2*tabsize)(stepPath.edge_);
                out << ',' << '\n';
                tab(indent+2*tabsize);
                out << "<Node> = ";
                printer(out, indent+2*tabsize)(stepPath.node_);
                out << ',' << '\n';
            }
            tab(indent+tabsize);
            out << ']' << '\n';
            tab(indent);
            out << '}';
        }

        void operator()(std::vector<ast::element> const& container) const {
            out << '[' << '\n';
            for(auto const& element: container) {
                tab(indent+tabsize);
                out << "<Element> = ";
                printer(out, indent+tabsize)(element);
                out << ',' << '\n';
            }
            tab(indent);
            out << ']';
        }

        void operator()(std::string const& text) const {
            out << '"' << text << '"';
        }

        void operator() (int const& n)          const {out << n;}
        void operator() (double const& n)       const {out << n;}
        void operator() (ast::and_ const& a)    const {out << "AND";}
        void operator() (ast::or_ const& a)     const {out << "OR";}
        void operator() (ast::eq_ const& a)     const {out << "==";}
        void operator() (ast::neq_ const& a)    const {out << "!=";}
        void operator() (ast::gt_ const& a)     const {out << ">";}
        void operator() (ast::lt_ const& a)     const {out << "<";}
        void operator() (ast::geq_ const& a)    const {out << ">=";}
        void operator() (ast::leq_ const& a)    const {out << "<=";}

        void operator() (bool const& b) const {
            if(b)
                out << "TRUE";
            else
                out << "FALSE";
        }

        void tab(unsigned spaces) const {
            for(int i = 0; i < spaces; i++) {
                out << ' ';
            }
        }
    }; // class printer


    // First visitor assigns VarIDs to variables, 
    // renames empty variables with _1, _2, etc and
    // checks for inconsistent calling of variables 
    // in SELECT and WHERE statements. Outputs the 
    // variable name to VarID map.
    class firstVisitor
        : public boost::static_visitor<StrIntMap>
    {

        private:

        StrIntMap VarIDMap;
        std::set<std::string> assignedVars;
        unsigned totObjects; 
        unsigned totUnassignedVars;

        public:

        // Constructor 
        firstVisitor() 
            : totObjects(0), totUnassignedVars(0) {};
        
        StrIntMap operator()(ast::root & r) {
            for(auto & lPattern: r.graphPattern_) {
                (*this)(lPattern);
            }
            boost::apply_visitor(*this, r.selection_);

            return VarIDMap;
        }

        StrIntMap operator()(ast::edge & edge) {
            if(edge.variable_.empty()) {
                edge.variable_ = "_" + std::to_string(totUnassignedVars+1);
                totUnassignedVars++;
            }
            else {
                assignedVars.insert(edge.variable_);
            }
            const auto ptr = VarIDMap.insert({edge.variable_, totObjects});
            if (ptr.second) {
                totObjects++;
            }
            return VarIDMap;
        }

        StrIntMap operator()(ast::node & node) {
            if(node.variable_.empty()) {
                node.variable_ = "_" + std::to_string(totUnassignedVars+1);
                totUnassignedVars++;
            }
            else {
                assignedVars.insert(node.variable_);
            }
            const auto ptr = VarIDMap.insert({node.variable_, totObjects});
            if (ptr.second) {
                totObjects++;
            }

            return VarIDMap;
        }

        StrIntMap operator()(ast::linear_pattern & lPattern) {
            (*this)(lPattern.root_);
            for(auto &sPath: lPattern.path_) {
                (*this)(sPath.edge_);
                (*this)(sPath.node_);
            }

            return VarIDMap;
        }

        StrIntMap operator()(std::vector<ast::element> & container) {
            for(auto & elem: container) {
                (*this)(elem);
            }
            return VarIDMap;
        }

        StrIntMap operator()(ast::element & elem) {
            // Check variable is present in match
            const bool found = assignedVars.find(elem.variable_) != assignedVars.end();
            if(!found) {
                throw ast::SelectionError(elem.variable_);
            }

            const auto ptr = VarIDMap.insert({elem.variable_ + "." + elem.key_ , totObjects});
            if (ptr.second) {
                totObjects++;
            }

            return VarIDMap;
        }

        StrIntMap operator()(ast::all_ const& a) {return VarIDMap;}

    }; // class firstVisitor


    // Second visitor assigns Entities to VarIDs
    // and checks for inconsistent entities present
    // at MATCH statement. Outputs a VarID to Entity map.
    class secondVisitor
        : public boost::static_visitor<IntEntMap>
    {
        StrIntMap idMap;
        IntEntMap id2Type;
        
        public:

        secondVisitor(StrIntMap & idMap) 
            : idMap(idMap) {}
        
        IntEntMap operator()(ast::root & r) {
            for(auto &lPattern: r.graphPattern_) {
                (*this)(lPattern);
            }
            boost::apply_visitor(*this, r.selection_);

            (*this)(r.where_);

            return id2Type;
            
        }

        IntEntMap operator()(ast::edge & edge) {
            
            auto search = id2Type.find(idMap.at(edge.variable_));
            if (search != id2Type.end()) {
                if (search->second != EDGE) 
                    throw ast::EntityError(edge.variable_);
            }
            id2Type[idMap.at(edge.variable_)] = EDGE;

            return id2Type;
        }

        IntEntMap operator()(ast::node & node) {
            auto search = id2Type.find(idMap.at(node.variable_));
            if (search != id2Type.end()) { // Found
                if (search->second != NODE) 
                    throw ast::EntityError(node.variable_);
            }
            id2Type[idMap.at(node.variable_)] = NODE;

            return id2Type;
        }

        IntEntMap operator()(ast::linear_pattern & lPattern) {
            (*this)(lPattern.root_);
            for(auto &sPath: lPattern.path_) {
                (*this)(sPath.edge_);
                (*this)(sPath.node_);
            }

            return id2Type;
        }

        IntEntMap operator()(std::vector<ast::element> & container) {
            for(auto &elem: container) {
                (*this)(elem);
            }

            return id2Type;
        }

        IntEntMap operator()(ast::element & elem) {
            // Check variable is present in match
            return id2Type;
            
        }

        IntEntMap operator()(boost::optional<ast::formula> & formula) {
            if(formula) {
                ast::formula realFormula = static_cast<ast::formula>(formula.get());
                boost::apply_visitor(*this, realFormula.root_);
                for (auto & sFormula: realFormula.path_) {
                    boost::apply_visitor(*this, sFormula.cond_);
                }
            }

            return id2Type;
            
        }

        IntEntMap operator()(ast::formula & formula) {
            boost::apply_visitor(*this, formula.root_);
            for (auto & sFormula: formula.path_) {
                boost::apply_visitor(*this, sFormula.cond_);
            }

            return id2Type;
        }

        IntEntMap operator()(ast::statement & stat) {
            (*this)(stat.lhs_);
            boost::apply_visitor(*this, stat.rhs_);

            return id2Type;
        }

        // Dummy leaves 
         IntEntMap operator()(ast::value & val)  {
            boost::apply_visitor(*this, val);
            return id2Type;
        }
        IntEntMap operator()(ast::all_ & a) {return id2Type;}
        IntEntMap operator()(std::string & text)  {return id2Type;}
        IntEntMap operator() (int & n)  {return id2Type;}
        IntEntMap operator() (double & n)  {return id2Type;}
        IntEntMap operator() (bool const& b) const {return id2Type;}
    }; // class secondVisitor


    // Third visitor retrieves the labels asociated to
    // each VarID where it corresponds. Returns a VarID (unsigned) 
    // to vector of Labels (str) map.
    class thirdVisitor
        : public boost::static_visitor<IntVectStrMap>
    {
        StrIntMap idMap;
        IntVectStrMap labelMap;

        public:

        // Constructor 
        thirdVisitor(StrIntMap & idMap)
            : idMap(idMap) {}
        
        IntVectStrMap operator()(ast::root const& r) {
            for(auto const& lPattern: r.graphPattern_) {
                (*this)(lPattern);
            }
            return labelMap;
        }

        IntVectStrMap operator()(ast::linear_pattern const& lPattern) {
            (*this)(lPattern.root_);
            for(auto &sPath: lPattern.path_) {
                (*this)(sPath.edge_);
                (*this)(sPath.node_);
            }

            return labelMap;
        }

        IntVectStrMap operator()(ast::edge const& edge) {
            unsigned varID = idMap.at(edge.variable_);
            auto found = labelMap.find(varID);
            if(found != labelMap.end())
                found->second.insert(found->second.end(), edge.labels_.begin(), edge.labels_.end());
            else 
                labelMap.emplace(std::make_pair(varID, std::vector<std::string>(edge.labels_)));

            return labelMap;
        }

        IntVectStrMap operator()(ast::node const& node) {
            unsigned varID = idMap.at(node.variable_);
            auto found = labelMap.find(varID);
            if(found != labelMap.end())
                found->second.insert(found->second.end(), node.labels_.begin(), node.labels_.end());
            else 
                labelMap.emplace(std::make_pair(varID, std::vector<std::string>(node.labels_)));

            return labelMap;
        }

        

    }; // class thirdVisitor

    // Fourth visitor retrieves the properties asociated 
    // to each VarID and checks consistency in the type
    // of values at WHERE and MATCH statements. Returns
    // a VarID to map of Key Values map. 
    class fourthVisitor
        : public boost::static_visitor<IntStrValMap>
    {
        StrIntMap idMap;
        IntStrValMap propertyMap;
        StrStrIntMap seenStatementValueType;

        public:

        // Constructor 
        fourthVisitor(StrIntMap & idMap)
            : idMap(idMap)
        {
        }
        
        IntStrValMap operator()(ast::root const& r) {
            for(auto const& lPattern: r.graphPattern_) {
                (*this)(lPattern);
            }
            (*this)(r.where_);
            boost::apply_visitor(*this, r.selection_);
            return propertyMap;
        }

        IntStrValMap operator()(ast::linear_pattern const& lPattern) {
            (*this)(lPattern.root_);
            for(auto &sPath: lPattern.path_) {
                (*this)(sPath.edge_);
                (*this)(sPath.node_);
            }

            return propertyMap;
        }

        IntStrValMap operator()(ast::edge const& edge) {
            for(auto & prop: edge.properties_) {
                propertyMap[idMap.at(edge.variable_)][prop.key_] =  prop.value_;
            }
            return propertyMap;
        }

        IntStrValMap operator()(ast::node const& node) {
            for(auto & prop: node.properties_) {
                propertyMap[idMap.at(node.variable_)][prop.key_] = prop.value_;
            }
            return propertyMap;
        }

        // Wrapper for the case of the optional where statement in ast::root 
        IntStrValMap operator()(boost::optional<ast::formula> const& formula) 
        {
            if(formula) {
                ast::formula realFormula = static_cast<ast::formula>(formula.get());
                boost::apply_visitor(*this, realFormula.root_);
                for (auto & sFormula: realFormula.path_) {
                    boost::apply_visitor(*this, sFormula.cond_);
                }
            }
            return propertyMap;
        }

        IntStrValMap operator()(ast::formula const& formula) {
            boost::apply_visitor(*this, formula.root_);
            for (auto & sFormula: formula.path_) {
                boost::apply_visitor(*this, sFormula.cond_);
            }

            return propertyMap;
        }

        

        IntStrValMap operator()(ast::statement const& stat) {

            // Check consistencies
            int storedWhich = boost::apply_visitor(whichVisitor(), stat.rhs_);
            if(storedWhich >= 0) { // Check type consistency against declared properties
                unsigned id_ = idMap.at(stat.lhs_.variable_);
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
                            if (storedWhich != foundIt->second) {
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

            return propertyMap;
        }


        IntStrValMap operator()(std::vector<ast::element> const& container) {
            unsigned varID;
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
            return propertyMap;
        }
            
        IntStrValMap operator()(ast::all_ const& a) {return propertyMap;}

        
        
    }; // class fourthVisitor

    // Fifth visitor retrieves the connections between
    // nodes with a corresponding edge. Returns an array
    // of 3 VarIDs: (1, 2, 3)  <-> (1) -[2]-> (3) .
    class fifthVisitor
        : public boost::static_visitor< connectVect >
    {
        StrIntMap idMap;
        connectVect connections;

        public:

        // Constructor 
        fifthVisitor(StrIntMap & idMap)
            : idMap(idMap) {}
        
        connectVect operator()(ast::root const& r) {
            for(auto const& lPattern: r.graphPattern_) {
                (*this)(lPattern);
            }
            return connections;
        }

        connectVect operator()(ast::linear_pattern const& lPattern) {
            unsigned id1, id2, id3;
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

            return connections;
        }
        
    }; // class fifthVisitor


} // namespace visitors


#endif