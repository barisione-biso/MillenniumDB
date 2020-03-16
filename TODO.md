- Comentarios Page y OrderedFile
- Crear test queries:
    - Busqueda de nodo por label:
        - largo string = 7, sin resultados
        - largo string = 7, 1 resultado
        - largo string = 7, varios resultados
        - largo string = 8, sin resultados
        - largo string = 8, 1 resultado
        - largo string = 8, varios resultados

    - Busqueda de edge por label:
        - largo string = 7, 1 resultado
        - largo string = 7, varios resultados
        - largo string = 8, 1 resultado
        - largo string = 8, varios resultados

    - Busqueda de nodo por labels:
        - largo string = <= 7 + largo string >= 8, varios resultados
        - largo string = <= 7 + largo string >= 8, 1 resultado

    - Busqueda de nodo por labels:
        - largo string = <= 7 + largo string >= 8, varios resultados
        - largo string = <= 7 + largo string >= 8, 1 resultado

    - Busqueda de nodo por property:
        - largo string = 7, 1 resultado
        - largo string = 7, varios resultados
        - largo string = 8, 1 resultado
        - largo string = 8, varios resultados

    - Busqueda de edge por label:
        - largo string = 7, 1 resultado
        - largo string = 7, varios resultados
        - largo string = 8, 1 resultado
        - largo string = 8, varios resultados

    - Busqueda de nodo por properties:
        - largo string = <= 7 + largo string >= 8, varios resultados
        - largo string = <= 7 + largo string >= 8, 1 resultado

    - Busqueda de nodo por properties:
        - largo string = <= 7 + largo string >= 8, varios resultados
        - largo string = <= 7 + largo string >= 8, 1 resultado


- sacar = default de los .h y pasalos al .cc
- add override to virtual functions overrided?

- Bulk Import: borrar ordered_files al terminar
- Buffer manager:
    - allocating more space if necesary
- Object File:
    - use disk if importing more than X nodes?
- Catalog:
    - Stop if having key for node/edge more than X different keys.
- Usar memcpy cuando sea posible (ordered file y bpt)