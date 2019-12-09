- Hacer varias búsquedas y medir tiempos

- Ordered file: checkear al cambiar el orden si en verdad es necesario
- Bulk Import: borrar ordered_files al terminar

- Crear Catalogo en bulk import
- FileManager: for each file open, have its open pages?
- Al hacer bulk import hash2id se va insertando 1 a 1, es posible mejorar esto?
- Posible optimización: en ordered file create_run usar merge sort y el insertion sort solo cuando el output_buffer hace flush
  ojo que al ordenar otra permutación se debe ordenar si o si
- Example queries
- Buffer manager:
    - allocating more space if necesary
- Importing graph:
    - remove "" to strings
    - add int support
- Object File:
    - use disk if importing more than X nodes?
- Catalog:
    - Stop if having key for node/edge more than X different keys.
- Usar memcpy cuando sea posible (ordered file y bpt)