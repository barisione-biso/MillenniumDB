#ifndef GRAMMAR__VISITORS_H
#define GRAMMAR__VISITORS_H

#include "ast.h"

#include <map>
#include <string>

// Typedefs for returning structs
typedef std::map<std::string, unsigned> str_int_map;
typedef std::map<unsigned, std::vector<std::string>> int_strs_map;
typedef std::map<unsigned, std::map<std::string, ast::value>> int_str_value_map;
typedef std::map<std::string, std::map<std::string, unsigned>> str_str_int_map;
typedef std::vector<std::array<unsigned, 3>> connect_vect;

#include "grammar/visitors/printer.h"
#include "grammar/visitors/assign_ids.h"
#include "grammar/visitors/assign_entities.h"
#include "grammar/visitors/assign_labels.h"
#include "grammar/visitors/assign_properties.h"
#include "grammar/visitors/assign_connections.h"
#include "grammar/visitors/value_getter.h"

#endif // GRAMMAR__VISITORS_H