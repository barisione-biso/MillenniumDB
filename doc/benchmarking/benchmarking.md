# Benchmarking

## Table of contents

1. [Introduction](#introduction)
* 1.1. [Benchmarking](#benchmark)
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
* 4.3. [Files and directories generated](#files-generated)
5. [Future work](#future-work)
* 5.1. [Incorporations of more engines](#engine-incorporation)






# 1. Introduction<a id="introduction"></a>

## 1.1 Benchmarking <a id="benchmark"></a>
Benchmarking refers to the evaluation of a system by a comparison to a standard. The following document describes the step to reproduce benchmarking.
## 1.2 sample files<a id="sample-files"></a>
Small sample files will be used to exemplify engine executions. These should not be used for benchmarking, just as a guide to check if everything works properly.
- `milleniumdb_db.txt` : Sample dataset file for MilleniumDB engine. <a id="mdb-db"></a>
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
- `sparql_db.ttl` :  Sample dataset file for SPARQL engines. <a id="sparql-db"></a>
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
- `milleniumdb_query.txt` : Sample query file for MilleniumDB engine. <a id="mdb-query"></a>
```
SELECT ?a, ?c
MATCH (?a)-[:knows]->(?b)-[:knows]->(?c)
```
- `sparql_query.txt` : Sample query file for SPARQL engines: <a id="sparql-query"></a>
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
- `blazegraph.properties`: Server configuration file. An example file is [this one](blazegraph.properties).

### 2.1.2 Jena<a id="jena-needed-files"></a>

The following files are needed to run Jena:
- `fuseki-server` : File needed to execute server. Available from https://jena.apache.org/download/, obtainable from apache-jena-fuseki-3.17.0.tar.gz. After decompressing, file is located in `apache-jena-fuseki-3.17.0/`
- `tdbloader2` : File needed to bulk import. Available from https://jena.apache.org/download/, obtainable from apache-jena-3.17.0.tar.gz. After decompressing, file is located in `apache-jena-3.17.0/bin/`

### 2.1.3 Virtuoso<a id="virtuoso-needed-files"></a>

The following files are needed to run Virtuoso:
- `virtuoso-t` : File needed to execute server. Available in https://github.com/openlink/virtuoso-opensource/releases, obtainable from virtuoso-opensource.x86_64-generic_glibc25-linux-gnu.tar.gz. After decompressing, file is located in `virtuoso-opensource/bin/`
- `isql` : File needed to execute bulk import. Available in https://github.com/openlink/virtuoso-opensource/releases, obtainable from virtuoso-opensource.x86_64-generic_glibc25-linux-gnu.tar.gz. After decompressing, file is located in `virtuoso-opensource/bin/`
- `virtuoso.ini` : Server configuration file. More information in `http://docs.openlinksw.com/virtuoso/dbadm/`. An example file is [this one](virtuoso.ini) (NOTE: "directory_with_binaries" and "directory_with_database" should be replaced accordingly.).

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

To use the benchmarking interface it is necessary to run `scripts/optional/tester.py <directory>`, where `<directory>` is the path to the directory where the files will be generated. An eaxmple execution would be `python3 scripts/optional/tester.py tmp`. This interface can compare already generated database instances and queries between each engine, and it can also create new instances of databases and queries to be compared.

## 4.1. Generate database and queries<a id="generate"></a>

If `<directory>` does not exist, the interface will generate both the database and the queries, according to the parameters established in the beginning of the file. NOTE: The database and queries generated are targeted to `OPTIONAL` operator testing, therefore it consists only of nodes and edges (no properties nor types). The parameters are the following:

- `HOTRUN`: Number of times to execute each query in hot run mode for the comparison. If `HOTRUN = N`, each query is run `N + 1` times and the first time is ignored for the comparison.
- `N_QUERIES`: Number of queries to be generated.
- `BLAZEGRAPH_ENDPOINT`: BlazeGraph query endpoint. Default is `http://localhost:9999/blazegraph/namespace/kb/sparql`.
- `JENA_ENDPOINT`: Jena query endpoint. Default is `http://localhost:3030/NAME/query`.
- `VIRTUOSO_ENDPOINT`: Virtuoso query endpoint. Default is `http://localhost:1122/sparql`
- `MIN_NODES`: Minimum amount of nodes in the database instance.
- `MAX_NODES`: Maximum amount of nodes in the database instance.
- `MIN_EDGES` = Minimum amount of edges (connections) in the database instance.
- `MAX_EDGES` = Maximum amount of edges (connections) in the database instance.
- `MIN_RELATIONS` = Minimum amount of different relations in the database instance. For example, if we had 3 relations, part of the instance could be:
    ```
    Q1 -[:knows]-> Q2
    Q6 -[:follows]->Q4
    Q3 -[:worksWith]->Q6
    ```
- `MAX_RELATIONS`: Maximum amount of different relations in the database instance.
- `N_VARS`: Maximum amount of variables involved in each query.
- `MIN_CHILDS`: Minimum amount of childs per node in the optional query  tree.
- `MAX_CHILDS`: Maximum amount of childs per node in the optional query tree.
- `MAX_DEPTH`: Maximum depth of the query tree.

## 4.2. Use custom database and queries<a id="custom"></a>
If `<directory>` does exist, the interface will bulk import the given database and execute the existing queries. For this to work the following needs to be set properly:
- `<directory>/dbs`: Directory containing the database files. Should contain:
    - `<directory>/dbs/mdb`: File with the database to be imported by MilleniumDB. The file should be in the correct [format](#mdb-db)
    - `<directory>/dbs/sparql.ttl`: File with the database to be imported by the rest of the engines. The file should be in turtle (`.ttl`) [format](#sparql-db).
- `<directory>/queries`: Directory containing the queries (each one in a file). Queries should be numbered starting from 0, and for each query the following file should exist (consider N the number of the query):
    - `<directory>/queries/mdb_N`: File with the query specified for the MilleniumDB engine, following the specified [format](#mdb-query).
    - `<directory>/queries/sparql_N`: File with the query specified for the other engines, following the SPARQL [format](#sparql-query).
## 4.3. Files and directories generated <a id="generated-files"></a>

When the benchmarking interface is executed a number of directories and files will be generated. If the execution involved the creation of the database and queries, the following will be created:
- `<directory>/dbs`: Folder with the database instance generated, in MilleniumDB format and SPARQL format.
- `<directory>/queries`: Folder with the queries generated, in MilleniumDB format and SPARQL format.

Additionally, the following will be created in any execution:
- `<directory>/blazegraph`: Directory with the files generated by the Blaze Graph engine.
- `<directory>/jena`: Directory with the files generated by the Jena engine.
- `<directory>/milleniumdb`: Directory with the files generated by the MilleniuMDB engine.
- `<directory>/virtuoso`: Directory with the files generated by the Virtuoso engine.
-`<directory>/query_log.csv`: File with the execution time of each query per engine. It also shows the number of results per query. Each query has a number of executions equal to the `HOTRUN` parameter established.
-`<directory>/mount_log.csv`: File with the execution time of each server start per engine.
- `<directory>/all_queries_time.png`: File that displays the execution time of all the query executions for each engine. If each query was run a `HOTRUN` number of times, it shows each one of them.
- `<directory>/average_query_time.png`: File that displays the average execution time of all the query executions per engine.
- `<directory>/n_results_per_query.png`: Displays the number of results per query.
- `<directory>/average_engine_time.png`: Displays the average query time per engine.

# 5. Future work<a id="future-work"></a>
Future work is proposed to extend the current benchmarking interface.
## 5.1. Incorporation of more engines<a id="engine-incorporation"></a>
The benchmarking interface should be extended to other engines for a better comparison. A list of proposed engines is the following:
- Stardog
- GraphDB
- AnzoGraph
- AllegroGraph
- MarkLogic
- Apache Rya

In order to incorporate more engines, the following steps should be followed:
1. Download the binary files needed for the execution of the bulk import process and the execution of the server, for the specific engine. If any other files are needed, they should also be created or downloaded. These files should be listed in the [needed files](#needed-files) section. If the file is a binary, its path should be added to the environment variables. If the file provides a configuration (e.g. `virtuoso.ini`) it should be added to the `doc/benchmarking/` directory.
2. Test the engine locally by using a small dataset and queries. It is recommended to use the [sample files](#sample-files) listed in this documentation. With the testing, the following information should be registered:
    - How to perform bulk import
    - How to run the server
    - How to execute queries in a specific endpoint (note: Some engines do not work with an endpoint and can only be executed locally).
    - How to stop the server
3. Create a wrapper for said engine following the format specified in the [wrapper interface](#wrapper-interface) section. If the engine works with a SPARQL endpoint, only the following methods should be overrode:
    - `bulk_import`
    - `start`

    If the engine does not work with a SPARQL endpoint (or if the engine has not been added to the SPARQLWrapper library), the following methods should be overrode additionally:
    - `query`
    - `process_results`
    - `stop`

4. Add the necessary information to the [benchmarking interface](#benchmarking-interface). Specifically:
    - If the engine has an endpoint, a constant should be added with the specified endpoint.
    - The engine should be added to `engine_list` list.
    - The Wrapper should be initialized.
