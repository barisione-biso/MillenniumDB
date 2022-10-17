# MillenniumDB + Sparql Refactor

Decide a "name" for namespaces, files, prefixes, etc. (RDF, SPARQL, Triple...)

## Table of contents
- [MillenniumDB + Sparql Refactor](#millenniumdb--sparql-refactor)
  - [Table of contents](#table-of-contents)
  - [1. doc/datatypes/](#1-docdatatypes)
  - [2. src/base/graph_object/](#2-srcbasegraph_object)
  - [3. src/base/ids/](#3-srcbaseids)
  - [4. src/base/query/sparql/](#4-srcbasequerysparql)
    - [Merge](#merge)
  - [5. src/base/query/](#5-srcbasequery)
  - [6. src/bin/](#6-srcbin)
    - [Merge](#merge-1)
  - [7. src/execution/](#7-srcexecution)
    - [Merge](#merge-2)
  - [8. src/import/](#8-srcimport)
    - [Merge](#merge-3)
  - [9. src/parser/query/sparql/](#9-srcparserquerysparql)
  - [10. src/query_optimizer/](#10-srcquery_optimizer)
  - [11. src/storage/](#11-srcstorage)
  - [12. src/tests/](#12-srctests)
  - [13. src/third_party/](#13-srcthird_party)
  - [14. CMakelists.txt](#14-cmakeliststxt)

## 1. doc/datatypes/
- Documentation for `DateTime` and `Decimal` types

## 2. src/base/graph_object/
- New types in `GraphObject`:
    1. `Boolean`
    2. `DateTime`
    3. `Decimal`
    4. `Iri(Inlined|External|Tmp)`
    5. `Literal(Datatype|Language)(Inlined|External|Tmp)`
- Shared types between `quad_model` and `rdf_model` are:
    1. `String(Inlined|External|Tmp)`

## 3. src/base/ids/
- New `ObjectId` (the same as the types in GraphObject except for Tmp)

## 4. src/base/query/sparql/
- New types for SparqlElement (this is a re-name for QueryElement):
    1. `Iri`
    2. `LiteralDatatype`
    3. `LiteralLanguage`
    4. `Literal`
    5. `Path(''|Alternatives|Atom|Sequence|KleeneStar|Optional|Denull)`
- Shared types between `GraphObject` and `SparqlElement` are:
    1. `Boolean`
    2. `DateTime`
    3. `Decimal`
    4. `Var` (Actually is scoped inside `src/base/query/`, but I would suggest to move it to `src/base/graph_object/`)
- `SparqlElement` has some new methods. It is important to notice that a `BlankNode` is just `Var` that can't be projected
- `SparqlElement` has its own visitors (`duplicate`, `to_graph_object` and `to_string`)

### Merge
- Decide if it is necessary to have both `QueryElement` and `SparqlElement`
    1. If we merge the `QueryElement` with `SparqlElement`, then the directories would be the same as `quad_model`.
    2. Otherwise, I would create the directories `query/sparql` and `query/millenniumdb` for scoping both types of queries.
- Check if some types could be merged: `Literal/String` and `Path/Path`

## 5. src/base/query/
- `StringManager` defines new methods: `get_prefix` (for IRIs) and `get_(datatype|language)` (for literals)

## 6. src/bin/
- New executable: `create_db_serd` (it can receive a prefixes file and uses the RDF classes)
- Modified executable: `server` (it uses `rdf_model` instead of `quad_model`)

### Merge
- For the `create_db_serd`, I would keep it separated from the other one, with a proper name (`create_db_(quad|rdf)`)
- Also, I would keep both servers separated (`server_(quad|rdf)`)

## 7. src/execution/
- `GraphModel` has a specific method for executing `SPARQL` queries and imports the specific operators for `SPARQL`
- New `BindingIter`: `Select` and `Where`

### Merge
- I would just add the new `BindingIter` out of the `binding_iter/sparql/` scope
- The operators will be discussed in another section

## 8. src/import/
- New permutaion on `DiskVector`
- New `import` for `rdf_model`
- New `PredicateStat` on `stats_processor`
- New methods in `Inliner`

### Merge
- `DiskVector` must be scoped for both `quad_model` and `rdf_model`
- Keep the `import` separated for both models
- `Inliner` methods for `iri`, `string` and `string5` should be merged and receive a parameter for the number of bytes to use. I would this for both `quad_model` and `rdf_model`

## 9. src/parser/query/sparql/
- On `grammar/`, has a new parser and lexer for `SPARQL`. Also there is a file called `triple.h`. It defines a triple, but I would think of moving this file to another directory, because it is used by Operators too.

- On `op/`, would think on creating both directories: `op/(mdb|sparql)`

- On `op_visitor/` -> merge with namespace (MDB and SPARQL)

- `QueryParser` -> rename with namespace (MDB and SPARQL)

## 10. src/query_optimizer/
- On `quad_model/`, remove `SPARQL` specific cases on `graph_object_visitor` and `exec_sparql` method on `quad_model.h`. Move `triple_plan` to another scope.
- Merge `rdf_model/*`

## 11. src/storage/
- `Catalog` has `String/Vector` methods
- Changed `FileManager` temporary files for `WSL1` compatibility
- `ObjectFile` implements methods for `prefixes`, `datatypes` and `languages`

## 12. src/tests/
- Full refactor of testing for MillenniumDB

## 13. src/third_party/
- Added `/serd/*` library
- Fixed debug execution warning for `antlr 4.9.3` (`ATNDeserializationOptions.(h|cpp)`)

## 14. CMakelists.txt
- Added `testing` configuration