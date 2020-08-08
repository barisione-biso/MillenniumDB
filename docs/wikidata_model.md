# Data Model
### Labels
A `Label` is just a string to describe a `Node`. Its only purpose is to filter the nodes when querying.

### Properties
A `Property` is a key/value pair where:
  - The key is a string.
  - The value may be an int, float, string, bool or a list of one of those types.
  - The keys have an expected value type (schema).

### Nodes
- A `Node` may have 0, 1 or many `Labels`.
- A `Node` may have 0, 1 or many `Properties`.
- A `Node` may be identified or anonymous
  - An `Identified Node` has a string as identifier.
  - An `Anonymous Node` only has an internal id.

### Connections
 A `Connection` is a relation between 4 objects (S,P,O,E)
- S represents the subject and must a `Node`.
- P represents the predicate and must be a `Node`.
- O represents the object and can be a `Node` or a `Value`.
  - maybe O should be only a `Node`
- E is an unique identifier for the `Connection` and must be an `Anonymous Node`

Sugerencia Domagoj:\
A `Connection` between S and O of type P, and E is an `Anonymous Node` to identify the connection.

# Indexes
- We use **internal_ids** of **8 bytes** to represent everything.
- The first byte of the internal_id indicates the type.
- Some types can be represented within the 7 bytes remaining (int, float, bool, short strings).
- Types that cannot be represented within 7 bytes will be saved in an object_file.
  - Those 7 bytes will point to its location in that file.
  - To get the object, we go to the pointed location and read 4 bytes to know its length, then we read the object.
- We have a hash table to transform strings to internal_ids.

### Label Index
- We have 2 B+Trees of pairs: NL and LN
  - NL is used to get the labels from a node_id
  - LN is used to search all nodes with certain label.

### Property Index
- We have 2 B+Trees of triples: KVN and NKV
  - KVN is used to search all nodes with certain key and value.
  - NKV can be used to get all properties of a node, or to get a certain property of a node.

### Connections Index
- We have 3 B+trees of quads: SPOE, OSPE, POSE and a table ESPO.
  - SPOE, OSPE, POSE allow to search any combination where E is not fixed.
  - ESPO allow to search when E is fixed.
    - Because E is an anonymous Node, its internal_id can be determined by the position in the table, so ESPO saves only 3 IDS (SPO) per connection.

# Wikidata Import

### Item/Property
- For each item or property we create one `Identified Node`.
- The `id` (e.g. Q60, P40) will be the identifier.
- The `type` (item or property) is added as a `Label`.
- `lastrevid` and `modified` are added as `Properties`.

#### Labels, Descriptions and Aliases
- For each `label`/`description`/`alias` we create a `Connection` where:
  - Subject is the item/property.
  - Predicate is a special Node `Label`, `Description` or `Alias`.
  - Object is the string value.
  - The connection has the property `language`.

#### Site Links
- For each `sitelink` we create a `Connection` where:
  - Subject is the item/property.
  - Predicate is a special Node `Sitelink`.
  - Object is an `Anonymous Node` with `site`, `title` and `url` as `Properties`.
  - For each `badge` we create a `Connection` where:
    - Subject is the anonymous node (object of sitelink)
    - Predicate is a special Node `Badge`
    - Object is the item (e.g. `Node(Q17437798)`)

#### Statements
- id (e.g. `"q60$5083E43C-228B-4E3E-B82A-4CB20A22A3FB"`) is ignored
- type is ignored (should be always `statement` so is redundant)
- The mainsnak creates a `Connection` where:
  - Subject is the item/property.
  - Predicate is the `property`
  - `snaktype` is a property, possible values: `"value"`, `"somevalue"` or `"novalue"`.
  - `datatype` is a property
  - Object depends on the `datavalue.type`
    - if `type` == `wikibase-entityid`
      - Object is the Node correesponding to the item.
    - if `type` == `string`
      - Object is the string value
    - if `type` == `globe-coordinate`
      - Object is a new `Anonymous Node` with label Globecoordinate
      - New node has `latitude`, `longitude`, `precision` and `globe` as properties
    - if `type` == `monolingual-text`
      - TODO:
    - if `type` == `quantity`
      - Object is a new `Anonymous Node` with label Quantity
      - New node has  `amount`, `upperBound`, `lowerBound` and `unit` as properties
    - if `type` == `time`
      - Object is a new `Anonymous Node` with label Time
      - New node has `time`, `timezone`, `calendarmodel` and `precision` as properties
  - The anonymous node representing the new `Connection` has the Label `Statement`
- `rank` is a property of the mainsnak connection
- Each qualifier creates a new `Connection` where:
  - Subject is the anonymous node created for the mainsnak `Connection`
  - Predicate is the `property`
  - Object depends on the `datavalue.type` with the same rules used for the mainsnak
  - The anonymous node representing the new connection has the Label `Qualifier`.
- Each reference is treated as a qualifier, but the anonymous node representing the connection has the Label `Reference`.
