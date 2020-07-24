# LDBC - Social Network Benchmark

## Import Times
| DB engine | Data | Time          | Size |
|:----------|:-----|--------------:|-----:|
| Neo4J     | SF1  |       ~22 sec | 1,6G |
| GraphDB   | SF1  |  2 min 16 sec | 4,8G |
| Rdf-3X    | SF1  |  6 min 38 sec | 3,2G |
| Neo4J 3.5 | SF10 |  2 min 28 sec | 16G  |
| GraphDB   | SF10 | 45 min 57 sec | 48G  |
| Rdf-3X    | SF10 | 1 hour 17 min | 34G  |


# Queries
Short Read (2)
```
SELECT ?m.id, ?m.content, ?m.creationDate, ?p.id, ?c.id, ?c.firstName, ?c.lastName
MATCH (:Person {id:17592186052613})<-[:HAS_CREATOR]-(?m:Message)-[:REPLY_OF]->(?p:Post),
      (?p)-[:HAS_CREATOR]->(?c)
```

Pair of persons that works at the same company.
```
SELECT ?p1.id, ?p2.id
MATCH (?p1 :Person)-[:WORK_AT]->(?c :Organisation {type:"company"})<-[:WORK_AT]-(?p2 :Person)
WHERE ?p1.id != ?p2.id
```

Pair of persons that studied at the same university.
```
SELECT ?p1.id, ?p2.id
MATCH (?p1 :Person)-[:STUDY_AT]->(?u :Organisation {type:"university"})<-[:STUDY_AT]-(?p2 :Person)
WHERE ?p1.id != ?p2.id
```

Pair of persons that works at the same company and studied at the same university.
```
SELECT ?p1.id, ?p2.id
MATCH (?p1 :Person)-[:WORK_AT]->(?c :Organisation {type:"company"})<-[:WORK_AT]-(?p2 :Person),
      (?p1)-[:STUDY_AT]->(?u :Organisation {type:"university"})<-[:STUDY_AT]-(?p2)
WHERE ?p1.id != ?p2.id
```

## Query Times

| Data | Query            | DB engine | Results    | Time        | Parsing+Optimizer Time |
|:-----|:-----------------|:----------|-----------:|------------:|-----------------------:|
| SF1  | short read       | Neo4J     |         36 |      332 ms |
| SF1  | short read       | GraphDB   |         36 |        6 ms | 117 ms
| SF1  | short read       | RDF-3X    |         36 |      370 ms |
| SF1  | same company     | Neo4J     |  1.153.178 |    1.717 ms |
| SF1  | same company     | GraphDB   |  1.153.178 |    1.508 ms |  1,3 ms
| SF1  | same company*    | RDF-3X    |  1.174.832 |    2.750 ms |
| SF1  | same university  | Neo4J     |    278.014 |      453 ms |
| SF1  | same university  | GraphDB   |    278.014 |      512 ms |  1,3 ms
| SF1  | same university* | RDF-3X    |    285.963 |      680 ms |
| SF1  | same comp & uni  | Neo4J     |     64.890 |    2.377 ms |
| SF1  | same comp & uni  | GraphDB   |     64.890 |      835 ms | 20,7 ms
| SF1  | same comp & uni* | RDF-3X    |            |  6m timeout |
| SF10 | short read       | Neo4J     |         45 |      382 ms |
| SF10 | short read       | GraphDB   |         45 |        7 ms | 119 ms
| SF10 | short read       | RDF-3X    |         45 |    3.080 ms |
| SF10 | same company     | Neo4J     | 51.742.296 |   72.906 ms |
| SF10 | same company     | GraphDB   | 51.742.296 |   85.250 ms | 1,3 ms
| SF10 | same company*    | RDF-3X    | 51.885.849 |  233.850 ms |
| SF10 | same university  | Neo4J     | 12.321.644 |   16.317 ms |
| SF10 | same university  | GraphDB   | 12.321.644 |   20.736 ms | 1,6 ms
| SF10 | same university* | RDF-3X    | 12.374.276 |   44.200 ms |
| SF10 | same comp & uni  | Neo4J     |  2.781.816 |   93.480 ms |
| SF10 | same comp & uni  | GraphDB   |  2.781.816 |   38.630 ms | 39,7 ms
| SF10 | same comp & uni* | RDF-3X    |            | 30m timeout |

## Query Analisys
### SF 10 same company
|                            | Results    | Execution Time | Parser/Optimizer Time |
|:---------------------------|-----------:|---------------:|----------------------:|
| normal                     | 51.742.296 |   85.250 ms    |   1,3 ms              |
| where ?p1 != ?p2           | 51.742.296 |   84.592 ms    |   1,3 ms              |
| where ?p1 != ?p2, select * | 51.742.296 |   54.003 ms    |   0,3 ms              |
| no where, select *         | 51.885.849 |   42.244 ms    |   0.3 ms              |
___

### SF 10 same university
|                            | Results    | Execution Time | Parser/Optimizer Time |
|:---------------------------|-----------:|---------------:|----------------------:|
| normal                     | 12.321.644 |      20.736 ms |   1,6 ms              |
| where ?p1 != ?p2           | 12.321.644 |      20.263 ms |   1,3 ms              |
| where ?p1 != ?p2, select * | 12.321.644 |      12.931 ms |   0,3 ms              |
| no where, select *         | 12.374.276 |       9.921 ms |   0,4 ms              |
___

### SF 10 same comp & uni time
|                            | Results    | Execution Time | Parser/Optimizer Time |
|:---------------------------|-----------:|---------------:|----------------------:|
| normal                     |  2.781.816 |   38.630 ms    |  39,7 ms              |
| where ?p1 != ?p2           |  2.781.816 |   35.736 ms    |  39,5 ms              |
| where ?p1 != ?p2, select * |  2.781.816 |   16.168 ms    |   6,5 ms              |
| no where, select *         |  2.897.090 |   15.008 ms    |   5,5 ms              |