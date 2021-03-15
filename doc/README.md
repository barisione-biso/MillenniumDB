# Overview

## What is Millenium DB?
MilleniumDB is a graph oriented database management system.

MilleniumDB was created because we though the existent graph models are were not enough to represent efficiently the real world data as a graph.

We created a new graph model and a new query language trying to be similar to the existent and more used.

Our main features are:
- [Work In Progress] Efficient property paths solving.
- [Work In Progress] Worst-case optimal algorithms for query solving.
- Easy migration from other graph models such as Neo4J or RDF (TODO: would be nice to have an auto-migration tool in the future).

## Data Model
Our data model is similar to the known *labeled property graph* model, but with a some key differences:
- Nodes may have a string Identifier.
- We don't make a hard distintion between nodes and edges:
    - Typically an edge defines a connection between 2 nodes, but our definition of conenction is a bit different: a connections is an object with 3 things:
        - An object source.
        - An object destination.
        - A set of types (zero, one or multiple types).
    - In simplefied terms we could say that our connections are like edges, but the source or destination may be another connection.
- We have a very flexible model, but imposing some restrictions can be good for query solving performance. For example, forcing all connections having one **Type** may simplify the indexing, resulting in faster query solving.

A full specification of our data model can be found [here](data_model.md).

## Query language
Our query language is similar to [cypher](https://neo4j.com/developer/cypher/).
- All queries need a `SELECT` and `MATCH` clause.
    - `MATCH` will find a graph pattern assign mappings to the variables declared, and `SELECT` will take a list of objects or its properties to be projected, or `SELECT *` to project all objects.
- Optionally, we may have `WHERE`, `ORDER BY` and `GROUP BY` clauses.
    - TODO: group by is not working yet

- The result of a query is a set of rows (TODO: bindings?), so we cannot have nested queries.

A query would look like:
```
SELECT ?x.name, ?x.age
MATCH (?x :Person)-[:Knows]->(John)
WHERE ?x.age > 60
```
Colloquially, this query is asking for the name and age of all persons that knows John and are older than 60.

For more details you can read the [query language specification](query_language.md) document.

## How to use it
### Getting the executable
- Option 1: compile from source.
    - TODO:

- Option 2: Download compiled binaries
    - TODO:

### Creating a database
**Important**: currently, once the database is created, it won't accept changes later.

A graph file would look like this:
```
Foo :FooLabel key_name_1:1 key_name_2:"hello"
Bar :BarLabel key_name_2:false
Foo->Bar :ConnectionType1 key_name_3:0.1
Foo->Bar :ConnectionType2
```

TODO: would be nice to have a realistic example database instead of random data we have now as an example We could use berlin or snb to create it?.

```
build/Release/bin/create_db tests/dbs/example-db.txt tests/dbs/example
```

### Querying a database
MilleniumDB uses the typical TCP server/client.

To start the server:
```
build/Release/bin/server [path/to/database_folder] -p [port]
```

To send a query:
```
build/Release/bin/query [path/to/query_file] -p [port]
```

## Internals
To know more details about how MilleniumDB works:
1. [How is the data stored](storage.md).
2. [How is the data accessed](internals/buffer_manager.md).
    - BufferManager
3. [Implementation limits](limits.md).
