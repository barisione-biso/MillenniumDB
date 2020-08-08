```
Our results suggested that while engines struggled with the singleton
property representation, no other representation was an outright winner
```
¿Why singleton properties performed that bad?
(generating excessive intermediate results)

# Wikidata reification
A claim can be viewed as a triple (S,P,O), but a claim might have multiple qualifiers (QP, QO).

## Standard Reification
For each claim we create a Claim Node (C) and we add 3 triples:
- C :subject S
- C :predicate P
- C :object O

And for each qualifier we add one triple:
- C QP QO

Claim     = 9 IDS\
Qualifier = 3 IDS

## Singleton Properties
For each claim we create a Claim Node (C) and we add 2 triples:
- S C O
- C :singletonPropertyOf P

And for each qualifier we add one triple:
- C QP QO

Claim     = 6 IDS\
Qualifier = 3 IDS

## N-ary Relations
For each claim we create a Claim Node (C) and we add 2 triples:
- S P1 C
- C P2 O

For each unique Property P:
- P1, :valueProperty, P
- P2, :statementProperty, P

And for each qualifier we add one triple:
- C QP QO

N Claims  = 2(N + P), where P = unique properties. Claim = 6~12 IDS\
Qualifier = 3 IDS

## Named Graphs
1 Quad for a Claim and 1 Triple/Quad for the qualifier
Claim     = 4 IDS\
Qualifier = 4 (maybe 3?) IDS

# Modeling Wikidata in a Property graph model

## Aproach 1
Creating an artificial node to represent the claim (C), and another node to represent the qualifier (Q).

- Connection(C, e1, S)
- Connection(C, e2, P)
- Connection(C, e3, O)
- Connection(C, e4, Q)
- Connection(Q, e5, QP)
- Connection(Q, e6, QV)
- LabeledConnection(:Subject,   C, e1, S)
- LabeledConnection(:Object,    C, e2, O)
- LabeledConnection(:Predicate, C, e3, P)
- LabeledConnection(:Qualifier, C, e4, Q)
- LabeledConnection(:QualProp,  Q, e5, QP)
- LabeledConnection(:QualValue, Q, e6, QV)
- EdgeLabel(e1, :Subject)
- EdgeLabel(e2, :Object)
- EdgeLabel(e3, :Predicate)
- EdgeLabel(e4, :Qualifier)
- EdgeLabel(e5, :QualProp)
- EdgeLabel(e6, :QualValue)

Claim     = 3\*3 + 3\*4 + 3\*2 = 27 IDS\
Qualifier = 3\*3 + 3\*4 + 3\*2 = 27 IDS

## Aproach 2
The claim is an edge (C) and it points to an artificial node (CN) via property. Analogously the qualifier is an edge(Q) pointing to its correesponding node (QN)

- Connection(S, C, O)
- Connection(CN, Q, QV)
- EdgeProperty(C, :Predicate, P)
- EdgeProperty(C, :Claim, CN)
- EdgeProperty(Q, :Predicate, QP)

To identify Claim Nodes from Qualifier Node we might also need a label
- Option 1:
    - NodeLabel(CN, :Claim)
    - NodeLabel(QN, :Qualifier)
- Option 2 (more space but could be faster when using quad):
    - EdgeLabel(C, :Claim)
    - EdgeLabel(Q, :Qualifier)
    - LabeledConnection(:Claim,     S, C, O)
    - LabeledConnection(:Qualifier, CN, Q, QV)

Claim = 1\*3 + 2\*3
- option 1: + 1\*2 = 11 IDS
- option 2: + 1\*2 + 1\*4 = 15 IDS

Qualifier = 1\*3 + 1\*3
- option 1: + 1\*2 = 8 IDS
- option 2: + 1\*2 + 1\*4 = 12 IDS

## Aproach 2.1
Also adding more quads, supposing fixed predicates will be common.

- EdgeLabel(C, :P)
- EdgeLabel(Q, :QP)
- LabeledConnection(:P, S, C, O)
- LabeledConnection(:QP, CN, Q, QV)

Claim += 6 IDS\
Qualifier += 6 IDS

There would be some redundancy with
- EdgeProperty(C, :Predicate, P)
- EdgeProperty(Q, :Predicate, QP)

If we eliminate them, how can we get the predicates using labels?

## Aproach 3
Nodes and edges are not longer different (like RDF)

- we could use any of the reifying methods, named graphs seems like the best.
- Property graph model is destroyed?
- Query language must change to express a Quad intention. Label is an IRI?
    - properties and connections are the same thing? (all properties use a reserver ID for anonymous edge)
    - (S)-[?e :P]->(O) transforms to quad (S, P, O, ?e)
    - [?e {k:v}] transforms to quad (?e, k, v, ?_) ?
    - or [?e {k:v}] transforms to triple (?e, k, v) ?
    - (N :L) Node Label is also a quad? (N, `_LABEL_`, L, ?_) or a pair?
    - can we have connections without a label?
    - can we have connections with multiple labels?
    - Neo4J: `A relationship must have exactly one relationship type.`
    - Delete graph and introduce prefixes?
    - 4 Permutations of the quad needs to be indexed (16 IDS per quad)
    - Leapfrog would require 4! = 24 permutations to answer all type of possible queries

```
(Q10) :Item key1:value1
(Q22) :Item key1:value2
(P30) :Item key1:value2
(Q10)-[P30]->(Q22) E1 :Label1 :Label2
(E1)-[P40]->(Q100)
(E1)-[P41]->(Q101)
```

Query:
```
(?s)-[?e :?p]->(?o)
(?e)-[:?q]->(?qv)
```


_ANONYMOUS_EDGE
(?n :person)-[:Knows]->(?m :Person)
(Knows)-[:SubClassOf]->(HumanRelationship)

Quad
(n, Knows, m, _id_8324854)
(n, 34534, m, _id_8324854)
