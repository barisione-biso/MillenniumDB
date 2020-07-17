# Cardinality estimation

## TODO:
- Cases when Label, Key or Value are assigned but unknown until query runtime ?

## Node Labels & Edge Labels

|Label|Node | Interpretation                                | Estimation
|:---:|:---:|:----------------------------------------------|:-----------
|  ✓  |  ✓  |Probability that a node has a certain label (L)| nodes with label L ÷ total nodes
|  ✓  |  ✕  |All nodes with certain label (L)               | nodes with label L
|  ✕  |  ✓  |*All labels of a certain node                   | total node labels ÷ total nodes
|  ✕  |  ✕  |*All pairs of (node, label)                     | total node labels

## Node Properties & Edge Properties

|Key  |Value|Node | Interpretation                                     | Estimation
|:---:|:---:|:---:|:---------------------------------------------------|:-----------
|  ✓  |  ✓  |  ✓  |Probability that a node has a certain key and value |nodes with key K ÷ (total nodes * `unique(K)`)
|  ✓  |  ✓  |  ✕  |All nodes with certain key/value                    |nodes with key K ÷ `unique(K)`
|  ✓  |  ✕  |  ✓  |**Probability that a node has a certain key K        |nodes with key K ÷ total nodes
|  ✓  |  ✕  |  ✕  |All nodes with certain key                          |nodes with key K
|  ✕  |  ✓  |  ✓  |--**not allowed**--                                 |
|  ✕  |  ✓  |  ✕  |--**not allowed**--                                 |
|  ✕  |  ✕  |  ✓  |*All propreties of a certain node                    |total node  properties / total nodes
|  ✕  |  ✕  |  ✕  |*All pairs of (node, key, value)                     |total node properties

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