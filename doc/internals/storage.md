# Storage

## IDS
TODO:

## Strings

## Persistent data structures:
* B+Tree
    - Ordered tuples of fixed size with O(log n) search.
        - Range search supported.
    - Used to index labels, properties and connections.
    - Leafs and directories are pages from the buffer manager.

* ObjectFile:
    - Contains all strings that cannot be codified inside the id separated by `\0`.
    - Doesn't use the buffer manager, all objects are in memory.
    - With the ID you can extract the object in O(1).

* HashTable:
    - Converts an string to an ID
    - Extendible hashing:
        - bucket is a page from the buffer manager.
        - the directory doesnt use the buffer manager and its always in memory.
    - Uses murmur3 hashing.
    - Works with hash collisions as long the collitions of a certain hash fits in a page (bulk import does not check this).

* RandomAccessTable:
    - enumerate nodes (only used in empty matches, e.g. `MATCH (?x)`).
    - edge_id => (from, to, type).
    - Data is divided by pages, uses the buffer manager.

* OrderedFile:
    - Used only in the import process to order records.
    - Uses a parallelized version of external merge sort.
    - Data is divided in pages, uses the buffer manager.
    - first you need to insert all the records one by one, then you can order the records in different permutations. Once ordered, you can access all the records one by one or create a B+Tree leaf with one page.

* TupleCollection
    - Used to order bindings (which differs from OrderedFile because the size is not known at compile time).
    - Data is divided in pages, uses the buffer manager.
    - TODO: delete file after using it, and dont harcode filename (otherwise ordering will fail)

* TODO: proximamente HashTable generico (sin ObjectFile)

## Examples
TODO: subsecciones para cada uno cuando se usa esa estructura. (ejemplos genericos no queries especificas)

## Quad Model
TODO: examples of queries and BPT used