+ Hacer que bulk import pueda hacer append a un arbol sin borrarlo
+ añadir test de fallos de semántica
    - hacer que se creen las excepciones correspondientes
+ Implementar "Quads virtuales"-> mover bits grafo al principio
    + keys, labels and values tienen mask del grafo
    - Idea: Blob para objetos muy largos, fuera de object file, su propio archivo

Dudas:
    - Qué pasa con el caso cuando hay muchos casos? El modelo con carpeta por cada grafo
      funcionaría mal.
    - Catalogo a traves de BufferManager o siempre en memoria?
    - Siguientes pasos
        - BufferManager nuevo
        - Query optimizer full
        - Algoritmos inteligentes
        - Operaciones de caminos (property paths, shortest paths, etc)
        - Operaciones de grafos (union, construct)
- Discutir solucion de tener todos los grafos en el mismo BPT



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
