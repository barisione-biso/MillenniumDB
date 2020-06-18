#!/bin/bash

BENCHMARK_FOLDER=/home/crojas/Documents/IMFD/Benchmark
JENA_FOLDER=$BENCHMARK_FOLDER/apache-jena-3.15.0
NEO4J_FOLDER=$BENCHMARK_FOLDER/neo4j-community-3.5.18
RDF3X_FOLDER=$BENCHMARK_FOLDER/gh-rdf3x
GRAPHDB_FOLDER=/home/crojas/GraphDB
QUERIES_FOLDER=$GRAPHDB_FOLDER/ldbc_sbn/short_reads

## APACHE JENA
# start fuseki server for Apache Jena
$JENA_FOLDER/fuseki/fuseki-server --loc=$JENA_FOLDER/snb /jena > /dev/null 2>&1 & disown
FUSEKI_PID=$!
echo Fuseki server runing at pid $FUSEKI_PID
for (( c=1; c<=10; c++ ))
do
   for file in $QUERIES_FOLDER/sparql/q*; do
        sleep 5
        ts=$(date +%s%N)
        $JENA_FOLDER/fuseki/bin/s-query --service=http://localhost:3030/jena/ --file=$file > /dev/null
        tt=$((($(date +%s%N) - $ts)/1000000))
        name=$(basename $file)
        echo "$name: $tt ms"
    done
done
kill $FUSEKI_PID

# $NEO4J_FOLDER/bin/neo4j start
# for (( c=1; c<=10; c++ ))
# do
#    for file in $QUERIES_FOLDER/cypher/q*; do
#         sleep 5
#         ts=$(date +%s%N)
#         cat $file | $NEO4J_FOLDER/bin/cypher-shell > /dev/null
#         tt=$((($(date +%s%N) - $ts)/1000000))
#         name=$(basename $file)
#         echo "$name: $tt ms"
#     done
# done
# $NEO4J_FOLDER/bin/neo4j stop

# for (( c=1; c<=10; c++ ))
# do
#    for file in $QUERIES_FOLDER/sparql/q*; do
#         sleep 5
#         ts=$(date +%s%N)
#         $RDF3X_FOLDER/bin/rdf3xquery $RDF3X_FOLDER/sbn $file > /dev/null 2>&1
#         tt=$((($(date +%s%N) - $ts)/1000000))
#         name=$(basename $file)
#         echo "$name: $tt ms"
#     done
# done

$GRAPHDB_FOLDER/build/Release/bin/server -d $GRAPHDB_FOLDER/test_files/db  > /dev/null 2>&1 & disown
GRAPH_DB_SERVER_PID=$!
echo Graph server runing at pid $GRAPH_DB_SERVER_PID
for (( c=1; c<=10; c++ ))
do
   for file in $QUERIES_FOLDER/graphdb/q*; do
        sleep 5
        ts=$(date +%s%N)
        $GRAPHDB_FOLDER/build/Release/bin/query $file > /dev/null
        tt=$((($(date +%s%N) - $ts)/1000000))
        name=$(basename $file)
        echo "$name: $tt ms"
    done
done
kill $GRAPH_DB_SERVER_PID