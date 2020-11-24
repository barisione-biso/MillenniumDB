# Order By

- Usar algoritmo similar al External Merge Sort:
  - Páginas son ordenadas con quicksort.
  - Se ejeucta en dos fases.
  - Use de Sort Page: Una versión más específica de Page que ayude al guardado y lectura de los datos.

En cuanto a parámetros, principalmente recibe:
  - BindingIdIter lhs: El conjunto de datos a ordenar -> **Cambiar nombre**.
  - BindingId input: Los ids de cada bind que determinan el orden **Cambiar nombre y tipo de dato Vector de VarId**.

Por definir: Guardo el valor a retornar en una variable y solo retorno true o false? O bien retorno y al finalizar retorno nullptr. **Next devuelve true o false**

 ## Sort Page

 Una página pero se estructura de la siguiente forma:
 - 10 bytes: 6 para el número de página, 2 para el tamaño del directorio y 2 para el espacio libre.
 - Directorio: Cada entrada usa 4 bytes: 2 apuntan al byte donde está el valor y los otros 2 su tamaño.
 - Espacio libre - valores

 Además tiene dos métodos:
  - write(x): Recibe un array de bytes / objeto (y lo transforma a bytes) y lo guarda en la página.

  - get(n): Retorna la n-esima tupla (se podría usar también [])

  Hay algunas dudas importantes:
  - Esto debe ser escalable, de momento almacenará los id de los binding, pero en un futuro la idea es que guarde los valores de manera directa. Por esto debería manejar el caso donde un valor no sea más grande que la página en sí mismo:

      - solución: usar un directorio de 5 bytes, donde el último indica si el final de la palabra está en esta página. (1 está en esta página, 0 no). Se lee una tupla hasta encontrar una página con que tenga dicho byte en 1. Si no está completo en una página, siempre estará en la siguiente (su uso es solo sort y no se implementa edit). **De momento no es necesario preocuparse de esto**
  - Con los ids no tenía problema en guardarlos, porque eran solo números, entonces el binding lo reconstruia al leer los valores y generar el objeto de nuevo (algo como ejecutar de nuevo su constructor). Primero ¿es correcta esta forma? (ver external merge sort) De no serlo cual sería la manera de recuperar el objeto desde los bytes.
  - Aun no me queda claro donde se conecta a la base de datos.**Usar la implementacion actual, preocupandose por el formato**


# Group By

Parámetros:
- input: Binding Id con el atributo por el que se agrupa -> ¿Será siempre uno? ¿O será múltiple? En ese caso podría ser un int con la posición en la tupla por la que se quiere agrupar
- lhs: Binding Id Iter. Aunque se le implementará un OrderBy en base al atributo.

Este algoritmo no debe retornar la tupla, sino más bien una tabla (un binding_id_iter por ej) que al recorrerlo me entregue las tuplas de ese grupo:

Ej: 
1 1 1 2 2 2 3 3

El next retornaría 3 veces una tabla para 1 (que me entrega 1 1 1), para 2 (2 2 2) y para 3 ( 3 3 ).
