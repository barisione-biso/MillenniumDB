+ poner en todas las clases con metodos virtuales un destructor virtual

+ Database configuration:
    + Folder name
    + Buffer size

+ Arreglar consulta:
    SELECT *
    MATCH (?n)
    WHERE ?n.name == "John"

+ Arreglar Memory Leak
- Import configuration:
    - Graph Name
- Usar memcpy cuando sea posible (ordered file y bpt)
- Reescritura de consulta (igualdades de where hacia match)

- Terminar de comentar clases de storage
- Hacer más tests de cosas en conjunto (ej: label+property)
- hacer diagramas de flujo de las siguientes operaciones:
    - bplustree search

____________________________

- cliente/servidor tcp boost::asio
- Presentar plan pointer swizzling

- Buffer manager:
    - allocating more space if necesary
- Object File:
    - use disk if importing more than X nodes?
- Catalog:
    - Stop if having key for node/edge more than X different keys.
- Usar BufferManager para OrderedFiles
- Al buscar property el value solo puede ser del valor dado. ¿Eliminar posibilidad de valores numéricos en match? ¿Eliminar solo float?
