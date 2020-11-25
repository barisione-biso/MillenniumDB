# Data Model

## Objects
There are 3 types of objects:
- **Identifiable nodes**: nodes asociated to a string e.g: "Q1"

- **Anonymous nodes**: nodes not asociated to a string. Can be used to represent a compose object (e.g. a wikidata time) or a resource not identified or unknown.

- **Connections**: object similar to an edge, that asociates 2 other objects with a direction (From and To). Not exactly a typical edge because we can have connections of connections.

## Connection Type
Connections have a (possibly empty) set of  **Identifiable nodes** as types. For wikidata we will assume the set is of size 1.

## Labels
**Identifiable nodes** and **Anonymous nodes** have a (possibly empty) set of strings as labels.
**Connections** don't have labels

## Properties
All 3 types of objects have a (possibly empty) set of key/values.
(e.g. "name":"John").
An object cannot have 2 properties with the same key.
