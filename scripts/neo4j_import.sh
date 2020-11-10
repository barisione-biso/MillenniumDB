#!/bin/sh
# TODO receive database name as parameter
$NEO4J_HOME/bin/neo4j-admin import --database wikidata3 \
 --nodes entity.csv \
 --nodes string.csv \
 --nodes time.csv \
 --nodes quantity.csv \
 --nodes url.csv \
 --nodes monolingual.csv \
 --nodes commons.csv \
 --nodes globe.csv \
 --nodes claims.csv \
 --relationships csv_claims_rel.csv \
 --relationships csv_qualifiers_rel.csv \
 --delimiter "," --array-delimiter ";" --skip-bad-relationships true

# TODO: echo message to create indexes
# After import must create index:
# CREATE INDEX FOR (n :Item) ON (n.id);
# CREATE INDEX FOR (n :Property) ON (n.id);

