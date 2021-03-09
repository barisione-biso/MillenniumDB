#ifndef BASE__PROPERTY_PATH_PARSER_H
#define BASE__PROPERTY_PATH_PARSER_H

#include <vector>

#include "base/parser/grammar/query/query_ast.h"

class PropertyPathParser {
public:
    PropertyPathParser();
    ~PropertyPathParser() = default;

    void insert(query::ast::LinearPattern);
    void generate_dfa();
    void compute();
};

#endif // BASE__PROPERTY_PATH_PARSER_H