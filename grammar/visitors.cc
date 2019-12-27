#include "ast.h"

#include <boost/variant.hpp>

using namespace visitors;

PrintAST :: printAST(std::ostream& out, unsigned indent = 0)
    : out(out), indent(indent) {}


void PrintAST :: operator()(ast::root const& r) const {
    out << '{' << '\n';
    tab(indent+tabsize);
    out << "<Select> = ";
    boost::apply_visitor(printAST(out, indent+tabsize), r.selection_);
    out << ',' << '\n';
    tab(indent+tabsize);
    out << "<GraphPattern> = [" << '\n';
    for (auto const& lPattern: r.graphPattern_) {
        tab(indent+2*tabsize);
        out << "<LinearPattern> = ";
        printAST(out, indent+2*tabsize)(lPattern);
        out << ',' << '\n';
    }
    tab(indent+tabsize);
    out << ']' << '\n';
    tab(indent+tabsize);
    out << "<Where> = ";
    printAST(out, indent+tabsize)(r.where_);
    out << '\n';
    tab(indent);
    out << '}' << '\n';
}


void PrintAST :: operator()(ast::formula const& formula) const {
    out << '{' << '\n';
    tab(indent+tabsize);
    out << "<Formula> = ";
    boost::apply_visitor(printAST(out, indent+tabsize), formula.root_);
    out << ',' << '\n';
    tab(indent+tabsize);
    out << "<Path> = [\n";
    for (auto const& sFormula: formula.path_) {
        tab(indent+2*tabsize);
        out << "<Op> = ";
        boost::apply_visitor(printAST(out, indent+2*tabsize), sFormula.op_);
        out << ',' << '\n';
        tab(indent+2*tabsize);
        out << "<Formula> = ";
        boost::apply_visitor(printAST(out, indent+2*tabsize), sFormula.cond_);
        out << ',' << '\n';
    }
    tab(indent+tabsize);
    out << ']' << '\n';
    tab(indent);
    out << '}';
}

void PrintAST :: operator()(ast::element const& elem) const {
    out << '{' << '\n';
    tab(indent+tabsize);
    out << "<Function> = ";
    printAST(out, indent+tabsize)(elem.function_);
    out << ',' << '\n';
    tab(indent+tabsize);
    out << "<Variable> = ";
    printAST(out, indent+tabsize)(elem.variable_);
    out << ',' << '\n';
    tab(indent+tabsize);
    out << "<Key> = ";
    printAST(out, indent+tabsize)(elem.key_);
    out << '\n';
    tab(indent);
    out << '}'; 
}

void PrintAST :: operator()(ast::statement const& stat) const {
    out << '{' << '\n';
    tab(indent+tabsize);
    out << "<LHS> = ";
    printAST(out, indent+tabsize)(stat.lhs_);
    out << ',' << '\n';
    tab(indent+tabsize);
    out << "<Comparator> = ";
    boost::apply_visitor(printAST(out, indent+tabsize), stat.comparator_);
    out << ',' << '\n';
    tab(indent+tabsize);
    out << "<RHS> = ";
    boost::apply_visitor(printAST(out, indent+tabsize), stat.rhs_);
    out << '\n';
    tab(indent);
    out << '}'; 
}

void PrintAST :: operator()(ast::value const& val) const {
    boost::apply_visitor(printAST(out, indent), val);
}

void PrintAST :: operator()(ast::edge const& edge) const {
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
        boost::apply_visitor(printAST(out, indent+2*tabsize), prop.value_);
        out << ',' << '\n';
    }
    tab(indent+tabsize);
    out << '}' << '\n';
    tab(indent);
    out << '}';
}

void PrintAST :: operator()(ast::node const& node) const {
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
        boost::apply_visitor(printAST(out, indent+2*tabsize), prop.value_);
        out << ',' << '\n';
    }
    tab(indent+tabsize);
    out << '}' << '\n';
    tab(indent);
    out << '}';
}

void PrintAST :: operator()(ast::linear_pattern const& lPattern) const {
    out << '{' << '\n';
    tab(indent+tabsize);
    out << "<Root> = ";
    printAST(out, indent+tabsize)(lPattern.root_);
    out << ',' << '\n';
    tab(indent+tabsize);
    out << "<Path> = [\n"; 
    for(auto const& stepPath: lPattern.path_) {
        tab(indent+2*tabsize);
        out << "<Edge> = ";
        printAST(out, indent+2*tabsize)(stepPath.edge_);
        out << ',' << '\n';
        tab(indent+2*tabsize);
        out << "<Node> = ";
        printAST(out, indent+2*tabsize)(stepPath.node_);
        out << ',' << '\n';
    }
    tab(indent+tabsize);
    out << ']' << '\n';
    tab(indent);
    out << '}';
}

void PrintAST :: operator()(std::vector<ast::element> const& container) const {
    out << '[' << '\n';
    for(auto const& element: container) {
        tab(indent+tabsize);
        out << "<Element> = ";
        printAST(out, indent+tabsize)(element);
        out << ',' << '\n';
    }
    tab(indent);
    out << ']';
}