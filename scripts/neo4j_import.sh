#!/bin/sh
$NEO4J_HOME/bin/neo4j-admin import --database exampledb2 \
 --nodes neo4j_nodes.csv \
 --relationships neo4j_edges.csv \
 --delimiter "|" --array-delimiter ";"