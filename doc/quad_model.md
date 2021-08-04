# Quad Model

The only restriction to the generic model is that **every connection must have one type**.

## Indices
- B+Trees:
    - Nodes:
        - (node)
    - Labels:
        - (node, label)
        - (label, node)
    - Properties:
        - (object, key, value)
        - (key, value, object)
    - Connections:
        - (from, to, type, connection)
        - (to, type, from, connection)
        - (type, from, to, connection)
        - (type, to, from, connection) redundant for IndexNestedLoopJoin but useful for leapfrog join.
    - Special case connections:
        - (from=to, type, connection)
        - (type, from=to, connection)
        - (to=type, from, connection)
        - (from, to=type, connection)
        - (from=type, to, connection)
        - (to, from=type, connection)
        - (from=to=type, connection)
- Table:
    - ConnectionTable: (connection, from, to, type)
    - The connection_id is not saved explicitly and is inferred by the position on the table.

- HashTable
    - Converts a string into an ID. Hash collitions are handled, but to many collisions are not supported. Actually if we have more than 186 (ObjectFileHashBucket::MAX_KEYS) strings with the same hash, the hashtable buckets will split indefinitely and the import will fail.

- ObjectFile
    - id to string

## 