- Hay tabla de nodos
    1) Q1 se hashea y se obtiene h_ID (o h_ID se obtiene inline si usa 7 bytes o menos)
    2) h_ID se busca en tabla de hash para obtener ID
    3) ID -> NodeTablePos no es nesesario

    - Labels son un subconjunto de los strings, no son nodos.
    - Connection Type != Label. Connection type si es en nodo.
    - Debería verificar que los nodos de las conexiones estan en la tabla
        - esto no se puede hacer hasta haber procesado todo el archivo (?)

Indexes:
- Node Table
    - only Identifiable

- Connection Table (e)-> From|To|Type

- B+Tree Connection
    - From|To|Type|e
    - To|Type|From|e
    - Type|From|To|e

    - Special cases (redundant index):
        - To = From
        - To = Type
        - From = Type
        - To = From = Type

- B+Tree Labels
    - Node|Label
    - Label|Node

- B+Tree Properties
    - (Node/Connection)|K|V
    - K|V|(Node/Connection)

Stats:
    - Total Identifiable Nodes
    - Total Anonymous Nodes
    - Total Connection Nodes (unique e)
    - Total Node (non-connection) keys
    - Total Connection keys
    - Total properties
