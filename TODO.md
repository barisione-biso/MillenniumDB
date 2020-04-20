+ funcionamiento cliente/servidor:
    + Retornar != 0 si hay error
    + soportar queries mas largas que el tamaño del buffer
+ añadir test de fallos de semántica
    - automatizar test con script bash
+ mutex en buffer manager/ pin/unpin
+ BUG graph not considered if all variables are unnamed in linear pattern

- Reescritura de consulta (igualdades de where hacia match)
- Ampliar el diagrama para explicar el prefijo de grafos
- Hacer más tests de cosas en conjunto (ej: label+property)
- hacer diagramas de flujo de las siguientes operaciones:
    - bplustree search

- Rama nueva con semántica default graph = union de todos los grafos
    - Nueva permutación para properties (key/node/value)
    - Cambiar creación de GraphScan en QueryOptimizer teniendo en cuenta la nueva semántica
    - Sacar prefijo de grafo de keys/labels/values al hacer bulk import
    - ¿Tipo pasa a estar antes que el grafo? -> tratar de reutilizar esos 2 bytes para valores
    - Merge de Ordered Files
    - Editar bulk import para que sepa que B+Trees necesitan merge y cuáles no



- Presentar plan pointer swizzling (cuántos punteros como máximo permitir?)
- Blob para objetos muy grandes, guardar fuera de object file, en su propio archivo.
- Buffer manager:
    - allocating more space if necesary ?
- Ordered File:
    - use disk if importing more than X nodes?
- Catalog:
    - Stop if having key for node/edge more than X different keys.
- Usar BufferManager para OrderedFiles ?
- Al buscar property el value solo puede ser del valor dado. ¿Eliminar posibilidad de valores numéricos en match? ¿Eliminar solo float?
- ver si string B-Tree puede ser útil https://algo2.iti.kit.edu/download/2014_-_Bachelor-Thesis_Fellipe_Lima.pdf
