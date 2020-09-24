# Cardinality estimation

## TODO:
- Cases when Label, Key or Value are assigned but unknown until query runtime ?

## Labels

|Label|Node | Interpretation                                | Estimation
|:---:|:---:|:----------------------------------------------|:-----------
|  ✓  |  ✓  |Probability that a node has a certain label (L)| nodes with label L ÷ total nodes
|  ✓  |  ✕  |All nodes with certain label (L)               | nodes with label L
|  ✕  |  ✓  |*All labels of a certain node                  | total node labels ÷ total nodes
|  ✕  |  ✕  |*All pairs of (node, label)                    | total node labels

## Properties

|Key  |Value| Obj | Interpretation                                     | Estimation
|:---:|:---:|:---:|:---------------------------------------------------|:-----------
|  ✓  |  ✓  |  ✓  |Probability that a object has a certain key/value   |objects with key K ÷ (total objects * `unique(K)`)
|  ✓  |  ✓  |  ✕  |All objects with certain key/value                  |objects with key K ÷ `unique(K)`
|  ✓  |  ✕  |  ✓  |**Probability that a object has a certain key K     |objects with key K ÷ total objects
|  ✓  |  ✕  |  ✕  |All objects with certain key                        |objects with key K
|  ✕  |  ✓  |  ✓  |--**not allowed**--                                 |
|  ✕  |  ✓  |  ✕  |--**not allowed**--                                 |
|  ✕  |  ✕  |  ✓  |*All propreties of a certain objects                |total object properties / total objects
|  ✕  |  ✕  |  ✕  |*All pairs of (object, key, value)                  |total object properties

- Unique values: `unique(K)`
    - how many different values a key K has.


## Connections
Node From|Node To|Edge | Interpretation                                         | Estimation
|:------:|:-----:|:---:|:-------------------------------------------------------|:-----------
|   ✓    |   ✓   |  ✓  |Probability that 2 nodes are connected by a certain edge|1 ÷ (total connections * (total nodes)<sup>2</sup>)
|   ✓    |   ✓   |  ✕  |All edges between 2 certain nodes                       |total connections ÷ (total nodes)<sup>2</sup>
|   ✓    |   ✕   |  ✓  |Probability that one node is connected by a certain edge|1 ÷ (total connections * total nodes)
|   ✓    |   ✕   |  ✕  |All connections of a node                               |total connections ÷ total nodes
|   ✕    |   ✓   |  ✓  |Probability that one node is connected by a certain edge|1 ÷ (total connections * total nodes)
|   ✕    |   ✓   |  ✕  |All connections of a node                               |total connections ÷ total nodes
|   ✕    |   ✕   |  ✓  |A single connection                                     |1
|   ✕    |   ✕   |  ✕  |All connections                                         |total connections

## Labeled Connections

Node From|Node To|Edge | Interpretation                    | Estimation
|:------:|:-----:|:---:|:----------------------------------|:-----------
|   ✓    |   ✓   |  ✓  |--**not allowed**--                |
|   ✓    |   ✓   |  ✕  |All edges between 2 certain nodes  |total connections with label L ÷ (total nodes)<sup>2</sup>
|   ✓    |   ✕   |  ✓  |--**not allowed**--                |
|   ✓    |   ✕   |  ✕  |All connections of a node          |total connections with label L ÷ total nodes
|   ✕    |   ✓   |  ✓  |--**not allowed**--                |
|   ✕    |   ✓   |  ✕  |All connections of a node          |total connections with label L ÷ total nodes
|   ✕    |   ✕   |  ✓  |--**not allowed**--                |
|   ✕    |   ✕   |  ✕  |All connections                    |total connections with label L