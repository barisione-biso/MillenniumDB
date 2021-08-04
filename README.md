# Millennium DB
MillenniumDB is a graph oriented database management system developed by the [Millennium Institute for Foundational Research on Data (IMFD)](https://imfd.cl/).

TODO: The objectives of this project

This project is in an early stage of development and is not production ready yet, as it may contain bugs.

This document is intended for new users that want to try MilleniumDB. For people that want to understand how the system works check out:
- [MillenniumDB Internals]()

And for people that want to contribute on the codebase see:
- [MillenniumDB Developer Guide]()

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

You should be able to build the in any x86-64 linux distribution, but our instructions are for distributions based on Ubuntu 18.04 or newer. Some distributions (like Ubuntu 18.04) might have repositories with a too old version of the Boost Library and you will need to manually install a version 1.71.0 or higher (https://launchpad.net/~mhier/+archive/ubuntu/libboost-latest may help).
Other linux distributions may need to install the prerequisites differenlty.

If you work on windows, you can use Windows Subsystem for Linux (WSL).

- Install prerequisites to compile:
    - `sudo apt update`
    - `sudo apt install git g++ cmake libboost-all-dev`

- Clone the repository:
    - if you use a ssh key:
        - `git clone git@github.com:MillenniumDB/MillenniumDB.git`
    - else:
        - `git clone https://github.com/MillenniumDB/MillenniumDB.git`
- Enter to the project folder:
    - `cd MillenniumDB`

- Build the project:
    - `cmake -H. -Bbuild/Release -DCMAKE_BUILD_TYPE=Release && cmake --build build/Release/`

# Data model

Everything in our graph model is an **Object**, and there are 4 different types of objects:

1. **Literals**: they are the classic basic types:
    - **Integer**
    - **String**
    - **Float**
    - **Boolean**
    - **TODO:** will arrays or composite objects (json-like) be supported in the future?

2. **Nodes**:
    they are objects that can have a an **identifier** set of **labels** and **properties**
    - `labels`: a (possibly empty) set of strings.
    - `properties`: a (possibly empty) set of pairs <key, value>. Where the key is a string and the value is a literal. A key cannot appear twice in the set.

    We have 2 types of nodes:

    - **Named Nodes**: they have a string as identifier when you add them into the database.

    - **Anonymous Nodes**: they don't have a string as identifier when you add them into the database. They will have an auto-generated identifier to direcly refeer to them later.


3. **Connections**: a connection is an object that relates other objects, having the following attributes:
    - An auto generated identifier.
    - `from`: any **Object** (including other connection object but not itself).
    - `to`: any **Object** (including other connection object but not itself).
    - `types`: a (possibly empty) set of **Named Nodes**.
    - `properties`: a (possibly empty) set of pairs <key, value>. Where the key is a string and the value is a literal. A key cannot appear twice in the set.


## Domain restrictions
- **Integer**: integers from -(2<sup>56</sup>-1) to 2<sup>56</sup>-1.

- **String**:
An important subset of strings are the property keys. They must match the regular expression `[A-Za-z][A-Za-z0-9_]*`. Property keys are not delimited by single nor double quotes.
The rest of the strings are delimited by double quotes (`""`) and they are a sequence of arbitrary length of any UTF-8 character, but some characters need to be encoded: [`\n`, `\t`, `\b`, `\f`, `\r`, `\/`, `\\`, `\"`].

- **Float**: same as in C language. Beware, you may lose precision.

- **Boolean** `true` or `false`.

- **Named Nodes**: they must match the regular expresion `[A-Za-z][A-Za-z0-9_]*`. There are exceptions for `true` and `false` because they are interpreted as booleans. Identifiers are case sensitive.

- **Anonymous Nodes** The auto-generated identifier will match the regular expresion `_a[1-9][0-9]*`.

- **Connections** The auto-generated identifier will match the regular expresion `_c[1-9][0-9]*`.

## Model constraints
The model presented before is very flexible, being that flexible comes with a downside of performance.
For that reason, we allow having multiple models, where each model meets the requirents of the generic data model and having some additional restrictions.

### Quad Model
Currently there is only one concrete model implemented. The only restriction to the generic model is that **every connection must have one type**. Thus connections can be saved as a tuple of 4 elements: (ConnectionID, FromID, ToID, TypeID).

### Future Models
We are planning to implement more data models in the future, for example a **TripleModel** where **every connection must have one type and we don't use connectionIDs**, as concequence, a Connection can't have any properties nor being connected. Thus connections can be saved as a tuple of 3 elements: (FromID, ToID, TypeID).



# Query language
TODO:

# Creating a database
The command to create a new database looks like this:
- `build/Release/bin/create_db [path/to/import_file] [path/to/new_database_folder]`

If you want to import a big database you should to specify a custom buffer size with the option `-b`. The parameter tells how many pages the buffer will allocate. Pages have a size of 4KB and the default buffer size is 1GB.

For instance, if you want to create a database into the folder `tests/dbs/example` using the example we provide in `tests/dbs/example-db.txt` having a 4GB buffer (4GB = 4KB * 1024 * 1024 and 1024 * 1024 = 1048576) you need to run:
- `build/Release/bin/create_db tests/dbs/example-db.txt tests/dbs/example -b 1048576`

To delete a database just manually delete the created folder.


# Querying a database
We implement the typical a client/server model, so in order to query a database, you need to have a server running and then send queries to it.
## Run the server
- `build/Release/bin/server [path/to/database_folder]`

## Execute a query
- `build/Release/bin/query [path/to/query_file]`
