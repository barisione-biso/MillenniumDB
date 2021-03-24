# The generic data model

Everything in our graph model is an **object**, and there are different subtypes of objects:
- **Nodes**:
Nodes have the following attributes:
    - `ID`: a (posibbly null) string. If the ID is not null,
    we denominate the node is an **identifiable node**, otherwise denominate the node as an **anonymous node**.
    - `labels`: a (possibly empty) set of strings.
    - `properties`: a (possibly empty) set of pairs <key, value>. A key cannot appear twice in the set.

- **Connections**: Connections have the following attributes:
    - `from`: an **Object**.
    - `to`: an **Object**.
    - `types`: a (possibly empty) set of **identifiable nodes**.
    - `properties`: a (possibly empty) set of pairs <key, value>. A key cannot appear twice in the set.

- **Literals**: literals are values of a certain type. Supported types are `integer`, `string`, `float` and `boolean`.

TODO: maybe is better to define `properties` as a partial function?


# Model constraints
The model presented before is very flexible, but most of the time we have some constrains we could take advantage on. For example if we know every **connection** will always have one **type**, we can make some optimizations on how we save the data on the disk and then we can save some operations when solving a query.

For that reason, we allow having multiples models depending on what constrains do we have. Depending on the model you use, you end up using different indexes to store the data and thus, each model has its own query solver (TODO: query optimizer? query planner?).


## Quad Model
Currently this is the only model implemented. The only restriction to the generic model is that every **connection** must have one **type**.
