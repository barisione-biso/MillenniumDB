+ añadir test de fallos de semántica
    - hacer que se creen las excepciones correspondientes
    - automatizar test con script bash
- Usar memcpy cuando sea posible (ordered file y bpt)
- comentar/refactor BPlusTree
- LIMIT en gramática y hacia proyección

- Reescritura de consulta (igualdades de where hacia match)
- Ampliar el diagrama para explicar el prefijo de grafos


- Hacer más tests de cosas en conjunto (ej: label+property)
- hacer diagramas de flujo de las siguientes operaciones:
    - bplustree search
- revisitar exceptions, usar logic_error solo cuando hay una precondición de código que se viola. Crear
  exceptciones adicionales si es necesario (ie: BufferManager)
____________________________

- Presentar plan pointer swizzling

- Blob para objetos muy grandes, guardar fuera de object file, en su propio archivo.
- Buffer manager:
    - allocating more space if necesary ?
- Object File:
    - use disk if importing more than X nodes?
- Catalog:
    - Stop if having key for node/edge more than X different keys.
- Usar BufferManager para OrderedFiles
- Al buscar property el value solo puede ser del valor dado. ¿Eliminar posibilidad de valores numéricos en match? ¿Eliminar solo float?
- ver si string B-Tree puede ser útil https://algo2.iti.kit.edu/download/2014_-_Bachelor-Thesis_Fellipe_Lima.pdf
