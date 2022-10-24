#pragma once

// TODO
enum class GraphObjectType {
    // The order for SPARQL Elements must follow the standard
    // src: https://www.w3.org/TR/rdf-sparql-query/#modOrderBy
    NULL_OBJ                  = 0,
    NOT_FOUND                 = 1,
    ANON                      = 2,
    IRI_INLINED               = 3,
    IRI_EXTERNAL              = 4,
    IRI_TMP                   = 5,
    STR_INLINED               = 6,
    STR_EXTERNAL              = 7,
    STR_TMP                   = 8,
    LITERAL_LANGUAGE_INLINED  = 9,
    LITERAL_LANGUAGE_EXTERNAL = 10,
    LITERAL_LANGUAGE_TMP      = 11,
    LITERAL_DATATYPE_INLINED  = 12,
    LITERAL_DATATYPE_EXTERNAL = 13,
    LITERAL_DATATYPE_TMP      = 14,
    DATETIME                  = 15,
    DECIMAL_INLINED           = 16,
    DECIMAL_EXTERNAL          = 17,
    DECIMAL_TMP               = 18,
    NAMED_INLINED             = 19,
    NAMED_EXTERNAL            = 20,
    NAMED_TMP                 = 21,
    EDGE                      = 22,
    PATH                      = 23,
    BOOL                      = 24,
    INT                       = 25,
    FLOAT                     = 26,
};
