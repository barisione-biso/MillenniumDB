# Benchmarking

## Table of contents

1. [Introduction](#introduction)
* 1.1. [Benchmarking](#benchmarking)
* 1.2. [Sample files](#sample-files)
* 1.3. [Important concepts](#important-concepts)
* 1.4 [Notes](#notes)
2. [Engines](#engines)
* 2.1. [Needed files](#needed-files)
    * 2.1.1. [BlazeGraph](#bg-needed-files)
    * 2.1.2. [Jena](#jena-needed-files)
    * 2.1.3. [Virtuoso](#virtuoso-needed-files)
* 2.2. [Engine execution](#engine-execution)
    * 2.2.1.  [Bulk import](#bulk-import)
        * 2.2.1.1. [BlazeGraph](#bg-bulk-import)
        * 2.2.1.2. [Jena](#jena-bulk-import)
        * 2.2.1.3. [Virtuoso](#virtuoso-bulk-import)
    * 2.2.2. [Start server](#start-server)
        * 2.2.2.1. [BlazeGraph](#bg-start-server)
        * 2.2.2.2. [Jena](#jena-start-server)
        * 2.2.2.3. [Virtuoso](#virtuoso-start-server)
    * 2.2.3. [Query server](#query-server)
    * 2.2.3. [Stop server](#stop-server)
3. [Wrapper Interface](#wrapper-interface)
* 3.1 [Environment variables](#environment-variables)
* 3.2 [How to use](#how-to-use)
4. [Benchmarking Interface](#benchmarking-interface)
* 4.1. [Generate database and queries](#generate)
* 4.2. [Custom database and queries](#custom)
5. [Future work](#future-work)
* 5.1. [Incorporations of more engines](#engine-incorporation)






# 1. Introduction<a id="introduction"></a>

## 1.1 Benchmarking <a id="benchmarking"></a>
Benchmarking refers to the evaluation of a system by a comparison to a standard. The following document describes the step to reproduce benchmarking.
## 1.2 sample files<a id="sample-files"></a>
Small sample files will be used to exemplify engine executions. These should not be used for benchmarking, just as a guide to check if everything works properly.
- `milleniumdb_db.txt` : Sample dataset file for MilleniumDB engine.
```
Q1 :Person
Q2 :Person
Q3 :Person
Q4 :Person
Q1->Q2 :knows
Q1->Q3 :knows
Q2->Q4 :knows
Q3->Q1 :knows
Q4->Q1 :knows
```
- `sparql_db.ttl` :  Sample dataset file for SPARQL engines.
```
@prefix : <http://www.example.com/> .
:Q1 :label :Person .
:Q2 :label :Person .
:Q3 :label :Person .
:Q4 :label :Person .
:Q1 :knows :Q2 .
:Q1 :knows :Q3 .
:Q2 :knows :Q4 .
:Q3 :knows :Q1 .
:Q4 :knows :Q1 .
```
- `milleniumdb_query.txt` : Sample query file for MilleniumDB engine.
```
SELECT ?a, ?c
MATCH (?a)-[:knows]->(?b)-[:knows]->(?c)
```
- `sparql_query.txt` : Sample query file for SPARQL engines:
```
PREFIX : <http://www.example.com/>
SELECT ?a  ?c
WHERE {
?a :knows ?b .
?b :knows ?c .
}
```
## 1.3 Important concepts<a id="important-concepts"></a>
For the execution of the different engines, it is important to know the following processes:
- Bulk import: Means to import the whole dataset from a file `<input>` into the engine. Usually done before the engine starts running. An example of an `<input>` file would be `sparql_db.ttl`. This process generates the files needed for the engine to run the server properly.
- Execute server: Creates an instance of a server of given engine, accessible by an endpoint.
- Query: Executes a query to retrieve data from the running server.
## 1.4 Notes<a id="notes"></a>
- `<file>` will refer to the path of a given file. It is **highly recommended** to work with absolute paths.
# 2. Engines<a id="engines"></a>
To benchmark it is necessary to test MilleniumDB against other engines. The chosen engines are:
- Blaze Graph
- Jena
- Virtuoso

To run the previous engines specific files are needed, which will be described next. How to execute the engines will also be described.

## 2.1 Needed files<a id="needed-files"></a>

### 2.1.1 Blaze Graph<a id="bg-needed-files"></a>

The following files are needed to run BlazeGraph:
- `blazegraph.jar`: File needed to execute server. Available in https://github.com/blazegraph/database/releases.
- `blazegraph.properties`: Server configuration file. An example file is [a this one](blazegraph.properties).
```

```

### 2.1.2 Jena<a id="jena-needed-files"></a>

The following files are needed to run Jena:
- `fuseki-server` : File needed to execute server. Available from https://jena.apache.org/download/, obtainable from apache-jena-fuseki-3.17.0.tar.gz. After decompressing, file is located in `apache-jena-fuseki-3.17.0/`
- `tdbloader2` : File needed to bulk import. Available from https://jena.apache.org/download/, obtainable from apache-jena-3.17.0.tar.gz. After decompressing, file is located in `apache-jena-3.17.0/bin/`

### 2.1.3 Virtuoso<a id="virtuoso-needed-files"></a>

The following files are needed to run Virtuoso:
- `virtuoso-t` : File needed to execute server. Available in https://github.com/openlink/virtuoso-opensource/releases, obtainable from virtuoso-opensource.x86_64-generic_glibc25-linux-gnu.tar.gz. After decompressing, file is located in `virtuoso-opensource/bin/`
- `isql` : File needed to execute bulk import. Available in https://github.com/openlink/virtuoso-opensource/releases, obtainable from virtuoso-opensource.x86_64-generic_glibc25-linux-gnu.tar.gz. After decompressing, file is located in `virtuoso-opensource/bin/`
- `virtuoso.ini` : Server configuration file. More information in `http://docs.openlinksw.com/virtuoso/dbadm/`. An example file is [a this one](virtuoso.ini) (NOTE: "directory_with_binaries" and "directory_with_database" should be replaced accordingly.).

## 2.2 Engine execution<a id="engine-execution"></a>

### 2.2.1 Bulk import<a id="bulk-import"></a>

Given a file `<input>` with the database instance (e.g.`sparql_db.ttl`):

**2.2.1.1 Blaze Graph**<a id="bg-bulk-import"></a>

`java -cp <blazegraph.jar> com.bigdata.rdf.store.DataLoader -namespace kb <blazegraph.properties> <input>`. Generates the needed files on the directory the command was executed.

**2.2.1.2 Jena**<a id="jena-bulk-import"></a>

`<tdbloader2> --loc <directory> <input>`. Generates the needed files on the directory specified by `<directory>`.

**2.2.1.3 Virtuoso**<a id="virtuoso-bulk-import"></a>

NOTE: as stated in the 'Needed files' segment, `directory_with_binaries` and `directory_with_database` should be replaced first in the `<virtuoso.ini>` file. `directory_with_binaries` should be the parent directory of `<tdbloader2>` (absolute path) and `directory_with_database` should be the parent directory of `<input>`. To run the bulk import process in virtuoso the following commands are needed:

- `<virtuoso-t> -c <virtuoso.ini> -f` to run the server.

- `<isql> 1111 dba dba exec="ld_dir(<directory_with_database>, '*.ttl', 'http://www.example.com')"` to set the files needed to be loaded.

[comment]: # (NOTE: It is necesary to have a file named `<input>.graph`, with `http://www.example.com` inside it. Otherwise, this command will not work.)

- `<isql> 1111 dba dba exec=rdf_loader_run();` to create the needed files for the database to run. Generates the needed files on the directory the command was executed.

- `<isql> 1111 dba dba exec=checkpoint;` to save the changes.

### 2.2.2 Start server<a id="start-server"></a>

**2.2.2.1 Blaze Graph**<a id="bg-start-server"></a>

`java -server -Xmx4g -jar <blazegraph.jar>` starts the server. The following endpoint must be used to query: `http://localhost:9999/blazegraph/namespace/kb/sparql`.

**2.2.2.2 Jena**<a id="jena-start-server"></a>

`<fuseki-server> --loc=<directory> /NAME` starts the server. The following endpoint must be used to query: `http://localhost:3030/NAME/query`. NOTE: `<directory>` should be the same directory specified in the bulk import process.

**2.2.2.3 Virtuoso**<a id="virtuoso-start-server"></a>

`<virtuoso-t> -c <virtuoso.ini> -f` starts the server. The following endpoint must be used to query: `http://localhost:1122/sparql`.

### 2.2.3 Query server<a id="query-server"></a>

For all the engines, you may access the endpoints to manually query over the data that was imported. However, it is recommended to automatize this process by using python3 and the SPARQLWrapper library. Given the query endpoint `<endpoint>` of any engine as stated in section 1.2.2, and given a query `<query>` with a valid SPARQL query (e.g. the text in `sparql_query.txt`), you can query the database as follows:

```python
from SPARQLWrapper import SPARQLWrapper, JSON
sparql_wrapper = SPARQLWrapper(<endpoint>)
sparql_wrapper.setQuery(<query>)
sparql_wrapper.setReturnFormat(JSON)
results = sparql_wrapper.query().convert()['results']['bindings']
```

### 2.2.3 Stop server<a id="stop-server"></a>

For all the engines, the server may be stopped by terminating the process. `htop` is recommended to check this. For virtuoso specifically, if by terminating the process the database is still running, the following command must be used: `<isql> localhost:1111 -K`

# 3. Wrapper interface<a id="wrapper-interface"></a>

In order to simplify the use of the different engines (including MilleniumDB), a wrapper interface was created, available in `scripts/optional/wrapper.py`. This makes all the results between the different engines comparable.

## 3.1 Environment variables<a id="environment-variables"></a>

To use this interface, a number of environment variables must be defined. For this, a `.env` file with the following may be used (NOTE: `<file>` must be the absolute path of the file.)

```
BLAZEGRAPH_JAR=<blazegraph.jar>
BLAZEGRAPH_PROPERTIES=<blazegraph.properties>
JENA_FUSEKI_SERVER=<fuseki-server>
JENA_TDBLOADER2=<tdbloader>
VIRTUOSO_T=<virtuoso-t>
VIRTUOSO_ISQL=<isql>
VIRTUOSO_INI=<virtuoso.ini>
MILLENIUMDB_CREATE=<create_db>
MILLENIUMDB_SERVER=<server>
MILLENIUMDB_QUERY=<query>
```

## 3.2 How to use<a id="how-to-use"></a>

The basic usage of the class is within `scripts/optional/wrapper.py`. A general example will be shown next (NOTE: `Wrapper` is an abstract class. Replace with non-abstract class such as `MilleniumDBWrapper`, `BlazeGraphWrapper`, `JenaWrapper` or `VirtuosoWrapper` as desired):

```python
wrapper = Wrapper(<endpoint>)           # Connects to the engine endpoint
wrapper.set_database_path(<directory>)  # sets the path where files will be generated
wrapper.bulk_import(<input>)            # imports the database and generates files
wrapper.start()                         # starts server
results = wrapper.query(<query>)        # queries the server given a file with the query
wrapper.stop()                          # stops the server
```

A working example is as follows, for the Virtuoso engine. In this case, `<virtuoso.ini>` is changed automatically. Change "sparql_db.ttl" and "sparql_query.txt" as needed:

```python
wrapper = VirtuosoWrapper("http://localhost:1122/sparql")
wrapper.set_database_path("virtuoso")
wrapper.bulk_import("sparql_db.ttl")
wrapper.start()
results = wrapper.query("sparql_query.txt")
wrapper.stop()
```

# 4. Benchmarking interface<a id="benchmarking-interface"></a>

## 4.1. Generate database and queries<a id="generate"></a>

## 4.2. Use custom database and queries<a id="custom"></a>

# 5. Future work<a id="future-work"></a>

# 5.1. Incorporation of more engines<a id="engine-incorporation"></a>
Extend to other engines, such as:
- Stardog
- GraphDB
- AnzoGraph
- AllegroGraph
- MarkLogic
- Apache Rya

Todo: Add how to incorporate more engines