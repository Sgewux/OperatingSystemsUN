# Práctica 1

## Estructura del dataset

El dataset **Genius Song Lyrics** contiene 11 columnas:

- **Title**: Título de la pieza (principalmente canciones).  
- **Tag**: Género.  
- **Artist**: Autor.  
- **Year**: Año de lanzamiento.  
- **Views**: Número de vistas.  
- **Features**: Artistas colaboradores.  
- **Lyrics**: Texto principal.  
- **Id**: Identificador único.  
- **language_cld3** y **language_ft**: Idiomas detectados por distintos modelos.  
- **language**: Idioma final si ambos modelos coinciden; *NaN* en caso contrario.

## Criterios de búsqueda

El campo utilizado para la indexación es *Title*, por ser el atributo más representativo de cada pieza.  
Opcionalmente, el campo *Artist* puede emplearse como filtro adicional en las búsquedas.


## Generación del índice hash

Para optimizar las búsquedas, se implementó un **índice hash en disco** mediante el programa `create_index.c`.  
Este lee el CSV y crea dos archivos binarios:

- **`hash_index.bin`**: tabla hash principal con los punteros a las listas de colisiones.  
- **`index_nodes.bin`**: nodos que almacenan la clave (*Title*), el desplazamiento del registro en el CSV y el puntero al siguiente nodo.

El proceso general consiste en:

1. Inicializar la tabla hash con valores vacíos (`-1`).  
2. Leer el CSV línea por línea.  
3. Calcular el hash del título y determinar su bucket.  
4. Crear un nodo con la información y enlazarlo en la lista correspondiente.  

De esta manera, se evita recorrer todo el archivo al buscar un registro.

---

## Búsqueda en el índice

Para localizar una canción:
1. Se calcula el hash del *Title* buscado.  
2. Se accede al bucket correspondiente en `hash_index.bin`.  
3. Se recorren los nodos encadenados en `index_nodes.bin` hasta encontrar la clave coincidente.  
4. Con el *offset* almacenado, se posiciona directamente en el CSV y se lee el registro completo.

Este enfoque permite realizar búsquedas mucho más rápidas que una exploración secuencial del dataset.


## Estructura general del proyecto

Para permitir la búsqueda al usuario final, se implementaron dos procesos un *cliente* y un *servidor* los cuales se comunican entre sí haciendo uso de *named pipes*.
