# Data Model

## Graphs
- A database is a set of one or more graphs.
- Each graph is a set of nodes and edges between those nodes.
- The default graph is defined as the union of all graphs.

## Nodes and Edges
- Edges connect nodes from the same graph.
- All edges have direction.
- Nodes and edges can have 0, 1 or multiple Labels.
- Nodes and edges can have 0, 1 or multiple Properties.
- Nodes and edges can't have more than one property with the same key.

## Label
- Non-empty string.

## Properties
- pair of key/value.
- key is a non-empty string.
- supported value types: `float`, `bool`, `string`, `int` (up to 56 bits).

# Import
- To import a graph, 2 files are needed, one with all the nodes and the other with all the edges.
- Each line will define a new node/edge.
- Nodes are prepresented by `(n)` where n is an ID (unique positive number). That ID won't be the same as the internal ID, and won't be saved in the database. If that ID needs to be saved it should be declared as property.
- Edges are represented by `(n1)->(n2)` or `(n1)<-(n2)`. Nodes `(n1)` and `(n2)` must be defined in the nodes file.
- After the node/edge is defined a list (possibly empty) of Labels is expected.
- After the label list, a list (possibly empty) of properties is expected.


## Node file example
```
(1) :Person :Student name:"John" age:20 graduated:false
(2) :Person :Professor name:"Joe" age:45
(3) :University name:"PUC"
(4) :University name:"U Chile"
```

## Edge file example
```
(1)->(2) :Knows
(1)<-(2) :Knows
(1)->(4) :Study_at since:2015
(3)->(2) :Works_at since:2019
```

# Database Limits
- Max Nodes: 2<sup>40</sup>
- Max Edges: 2<sup>40</sup>
- Max Graphs: 2<sup>16</sup>
- Max string length: 2<sup>32</sup> (not definitive)
- B+Tree pages: 2<sup>32</sup> (not definitive)
- Max hash (murmur) collitions: TODO:

# Query Language
TODO:
## Select
## Match
## Limit