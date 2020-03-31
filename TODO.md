+ Import configuration:
    + Graph Name
    + Folder Name
+ keyword ON para especificar grafo
+ Actualizar catalogo para tener nombres de grafos
+ añadir test de fallos de semántica
    - hacer que se creen las excepciones correspondientes
- Usar memcpy cuando sea posible (ordered file y bpt)
- Reescritura de consulta (igualdades de where hacia match)

- Terminar de comentar clases de storage
- Hacer más tests de cosas en conjunto (ej: label+property)
- hacer diagramas de flujo de las siguientes operaciones:
    - bplustree search
- revisitar exceptions, usar logic_error solo cuando hay una precondición de código que se viola. Crear
  exceptciones adicionales si es necesario (ie: BufferManager)
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
