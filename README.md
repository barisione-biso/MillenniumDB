# Millennium DB
MillenniumDB is a graph oriented database management system developed by the [Millennium Institute for Foundational Research on Data (IMFD)](https://imfd.cl/).

MillenniumDB was created because we though the existent graph models were not enough to represent efficiently the real world data as a graph.

Our main objective with this project is to be a fully functional, easy-to-extend system that serves as the basis for testing new techniques and algorithms related to databases and graphs.

## Main features
- A flexible and more expressive graph [data model](#data-model).
- Efficient property paths solving.
- Worst case optimal join algorithm.

This project is in an early stage of development and is not production ready yet, as it may contain bugs. Also, there are some features we think are important for a graph database and are not supported yet. However we want them to be supported in the future:

- A database can't be modified after its created.
- Only [well-designed patterns](https://users.dcc.uchile.cl/~jperez/papers/tods-a16-perez.pdf) are supported when using the `OPTIONAL` operator.
- More datatypes for properties values, e.g: dates, points and lists.
- Filters only support basic operations (<, <=, ==, != >=, >). We would like to support pattern matching and functions.


This document is intended for new users that want to try MillenniumDB. People that want to understand how the system works should check out:
- TODO: [MillenniumDB Internals](#)

And for people that want to contribute on the codebase see:
- TODO: [MillenniumDB Developer Guide](#)

___
# Table of Contents
- [Project build](#project-build)
- [Data model](#data-model)
- [Query language](#query-language)
- [Creating a database](#creating-a-database)
- [Querying a database](#querying-a-database)
  * [Run the server](#run-the-server)
  * [Execute a query](#execute-a-query)
___
# Project build

MillenniumDB should be able to be built in any x86-64 linux distribution.
If you work on windows, you can use Windows Subsystem for Linux (WSL).

1. Install prerequisites to compile c++:

    - Gcc version 8.1 or newer
    - Cmake version 3.12 or newer

    For distributions based on **Ubuntu 20.04 or newer** this can be done by running:
    - `sudo apt update`
    - `sudo apt install git g++ cmake libssl-dev`

     Other linux distributions may need to install the prerequisites differently. Some distributions might have repositories with too old versions and the project won't compile, in that case you'll need to manually install the appropriate versions.

2. Clone this repository and enter to the 'MillenniumDB' folder. Set `MDB_HOME` as the project root folder directory.
    - `git clone git@github.com:MillenniumDB/MillenniumDB-Dev.git`
    - `cd MillenniumDB-Dev`
    - `export MDB_HOME=$(pwd)`

3. Install the Boost Library version 1.81.0
    - Download the file `boost_1_81_0.tar.gz` from `https://www.boost.org/users/history/version_1_81_0.html` and extract it.
        - `tar -xf boost_1_81_0.tar.gz`
    - Enter the boost directory
        - `cd boost_1_81_0/`

    - `./bootstrap.sh --prefix=$MDB_HOME/third_party/boost_1_81`
    - `./b2 --prefix=$MDB_HOME/third_party/boost_1_81`
    - `./b2 install`

4. Build the project:
    - `cmake -B build/Release -D CMAKE_BUILD_TYPE=Release && cmake --build build/Release/`

# Data model
Our data model is similar to the known *labeled property graph* model. In simplified terms we could say that edges were extended such that the source or destination may be another edge. To be more precise, below is the full specification.

Everything in the graph model is an **Object**, and there are 4 different types of objects:

1. **Literals**: they are the classic basic types:
    - **Integer**
    - **String**: also we have 3 important subsets of strings
        - **Names**
        - **Keys**
        - **Labels**

    - **Float**
    - **Boolean**

2. **Nodes**:
    they are objects that can have the following attributes:
    - `ID`.
    - `labels`: a (possibly empty) set of **labels**.
    - `properties`: a (possibly empty) set of pairs **<Key, Literal>**. The set cannot have 2 properties with the same key.

    We have 2 types of nodes depending on the ID:

    - **NamedNodes**: they have a **Name** as identifier when you add them into the database.

    - **AnonymousNodes**: they don't have a name as identifier when you add them into the database. They will have an auto-generated identifier to directly refer to them later.

3. **Edges**: an edge is an object that relates other objects, having the following attributes:
    - `ID` (always auto-generated in edges).
    - `from`: an **Object** (including other edge object but not itself).
    - `to`: an **Object** (including other edge object but not itself).
    - `types`: a (possibly empty) set of **NamedNodes**.
    - `properties`: a (possibly empty) set of pairs **<Key, Literal>**. The set cannot have 2 properties with the same key.


## Model constraints
The abstract model presented before is very flexible, but being that flexible may come with a downside of performance.
For that reason, we allow having multiple concrete models, where each model meets the requirements of the generic data model with some additional restrictions.

### Quad Model
Currently **QuadModel** is the only one implemented. The only restriction to the generic model presented before is that **every edge must have one type**. Thus edges can be saved as a tuple of 4 elements: <EdgeID, FromID, ToID, TypeID>.

### Future Models
We are planning to implement more data models in the future, for example a **TripleModel** where **every edge must have one type and we don't use the EdgeID**, as consequence, an edge can't have properties and nested edges can't exist. Thus edges can be saved as a tuple of 3 elements: <FromID, ToID, TypeID>. This model would allow to represent RDF-like data (subject=from, object=to, predicate=type) without using additional bytes for the EdgeID.


# Query language

A query would look like:
```
// This query is asking for the age and name of people
// that knows John having between 60 and 70 years old,
// ordered by their age (ascending) and name (descending).
MATCH (?x :Person)-[Knows]->(John)
WHERE ?x.age >= 60 AND ?x.age <= 70
ORDER BY ?x.name DESC, ?x.age ASC
RETURN ?x.age, ?x.name
LIMIT 1000
```

Let's analyze line by line

- The first 3 lines are comments. A comment starts with a `//` and finish at the end of the line.

- The next line is a MATCH clause. Every query must have a MATCH clause at the beginning. The MATCH clause is followed by a **graph pattern**. To define a graph pattern we need to define some other smaller concepts. First we define the **node pattern**. The most basic **node pattern** looks like this:
    - `()`

    And there are some things you can add to a **node pattern**:

    - A **node identifier** that refers to a specific node or literal.
        | **node type** | example node identifier |
        | :--- | ---: |
        |NamedNode|`(John_Doe)`|
        |AnonymousNode|`(_a123)`|
        |String Literal|`("some string")`|
        |Boolean Literal|`(true)`|
        |Integer Literal|`(123)`|
        |Float Literal|`(3.14)`|

    - If you are not using a **node identifier**, you can add a **variable** to bind the node.
        - `(?x)`

    - A list of labels (after the variable or node identifier if they are present)
        - `(:Label)`
        - `(:Label1 :Label2)`
        - `(?x :Label)`
        - `(?x :Label1 :Label2 :Label3)`
    - A set of properties (a the end)
        - `({key:"value"})`
        - `(?x {key1:"value1", key2:"value2"})`
        - `(?x :Label {key:"value"})`


    Two **node patterns** can be connected to each other vie **edges**, **edges** always have a **direction**:
    - `(?x)->(?y)`
    - `(?y)<-(?z)`

    And similar as in **node patterns**, **edges** may contain other some things:
    - An **edge variable** to bind the edge object:
        - `(?x)-[?e]->(?y)`
    - Or instead of a **edge variable**, a **fixed edge**:
        - `(?x)-[_e123]->(?y)`
    - A **type variable** (after the edge variable or fixed edge if they are present):
        - `(?x)-[:?t]->(?y)`
        - `(?x)-[?e :?t]->(?y)`
    - Or instead of a **type variable**, a **fixed type** (after the edge variable or fixed edge if they are present):
        - `(?x)-[:Type1]->(?y)`
        - `(?x)-[?c :Type2]->(?y)`
    - A set of properties (at the end)
        - `(?x)-[{key:"value"}]->(?y)`
        - `(?x)-[?c :Type {key:"value"}]->(?y)`

    Then a **linear pattern** is a set of one or more **node patterns** linked by edges (TODO: or property paths):
    - `(?x :Person)-[:Knows]->(?y)<-[Knows]-(John)`

    A set of one or more **linear patterns** (separated by comma) forms a **simple graph pattern**
    - `(?x :Person)-[:Knows]->(?y)<-[:Knows]-(John), (?y)-[:LivesIn]->(Chile)`

    Finally, a **graph pattern** is defined as follows:
    - A **simple graph pattern** is a **graph pattern**.
    - If `GP1` and `GP2` are **graph patterns**, `GP1 OPTIONAL { GP2 }` is a **graph pattern**

- TODO: WHERE specification is incomplete. The next line is a WHERE clause. A query may not have a WHERE clause. The WHERE clause filters the results obtained by the MATCH clause according to certain **condition**. An **atomic condition** looks like:
    - `?x == ?y`
    - `?x == ?y.key`
    - `?x == "literal"`
    - `?x.key == ?y`
    - `?x.key1 == ?y.key2`
    - `?x.key1 == "literal"`

    and `==` may be replaced with the following operators `<, <=, !=, >=, >` and `"literal"` can be replaced with a literal of any type. Complex conditions are defined as:
    - An **atomic condition** is a valid **condition**.
    - If `C1` is a valid condition, `NOT C1` and `(C1)` are valid conditions.
    - If `C1` and `C2` are valid conditions. `C1 AND C2` and `C1 OR C2` are valid conditions.

    The operator precedence is as usual: `()` > `NOT` > `AND` > `OR`.

- The next line is an ORDER BY clause. A query may not have a ORDER BY clause. You can specify how the order works with the keywords `ASC`/`ASCENDING` and `DESC`/`DESCENDING` after each element. If the order is not specified the default is `ASCENDING`

- The next line is a RETURN clause. Every query must have a RETURN clause. This clause specify which objects or object properties will be returned. Return clauses look like this:
    - `RETURN *`
    - `RETURN ?x`
    - `RETURN ?x.key`
    - `RETURN ?x, ?y.key, ?z`

- The last line is a LIMIT clause. A query may not have a LIMIT clause. A LIMIT clause gives an upper bound on the number of results returned.

- TODO: link to example queries


# Creating a database
The command to create a new database looks like this:
- `build/Release/bin/create_db [path/to/import_file] [path/to/new_database_folder]`

For instance, if you want to create a database into the folder `tests/dbs/example` using the example we provide in `tests/dbs/example-db.txt` you need to run:
- `build/Release/bin/create_db tests/dbs/example-db.txt tests/dbs/example`

To delete a database just manually delete the created folder.


# Querying a database
We implement the typical a client/server model, so in order to query a database, you need to have a server running and then send queries to it.
## Run the server
- `build/Release/bin/server [path/to/database_folder]`

## Execute a query
- `build/Release/bin/query < [path/to/query_file]`
