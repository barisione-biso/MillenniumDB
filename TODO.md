- Rama nueva con semántica default graph = union de todos los grafos
    + Nueva permutación para properties (key/node/value)
    + Cambiar creación de GraphScan en QueryOptimizer teniendo en cuenta la nueva semántica
    + Sacar prefijo de grafo de keys/labels/values al hacer bulk import
    + ¿Tipo pasa a estar antes que el grafo? -> tratar de reutilizar esos 2 bytes para valores
    + GraphIds parten de 1
    - Merge de Ordered Files
    - Editar bulk import para que sepa que B+Trees necesitan merge y cuáles no
    - Editar catálogo en importacion: siempre modificar también el default graph
    - Node Enumeration: caso especial si es default graph

- permitir añadir datos a grafo existente -> otro ejecutable?

- Rehacer dependencia de objetos globales:
    - sacar nifty counter
    - Modelo los inicializa y destruye?
    - Ejecutables inicializan Modelo. Usar factory para parámetros?
- Testear importacion de diferentes grafos en ambos modelos
    - partir con grafo grande y añadir chicos.
    - partir con grafo chico y añadir grande.

- automatizar test con script bash
- Reescritura de consulta (igualdades de where hacia match)
- Ampliar el diagrama para explicar el prefijo de grafos
- Hacer más tests de cosas en conjunto (ej: label+property)
- hacer diagramas de flujo de las siguientes operaciones:
    - bplustree search

- Importar varios grafos en el mismo bulk_import
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
