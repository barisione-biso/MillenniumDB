#!/bin/sh
$NEO4J_HOME/bin/neo4j-admin import --database exampledb \
 --nodes nodes.csv \
 --relationships edges.csv \
 --delimiter "|" --array-delimiter ";"