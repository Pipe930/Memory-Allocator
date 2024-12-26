#include "utils.cpp"

/**
 * @brief Asigna un bloque de memoria dinámico del heap.
 *
 * Esta función implementa un mecanismo básico de asignación de memoria similar a `malloc`. 
 * Gestiona el heap mediante una lista enlazada de bloques de memoria. Si no se encuentra 
 * un bloque libre adecuado, solicita espacio adicional al sistema. 
 *
 * @param size Tamaño en bytes del bloque de memoria solicitado.
 * @return Puntero al espacio de memoria asignado, o `nullptr` si no se puede asignar memoria.
 * 
 * @details
 * - Si el tamaño solicitado es menor o igual a 0, se retorna `nullptr` inmediatamente.
 * - El tamaño se alinea para garantizar que cumple con los requisitos de alineación del sistema.
 * - Si esta es la primera llamada a `malloc`, inicializa el heap solicitando espacio al sistema.
 * - Si el heap ya está inicializado, busca un bloque libre adecuado; si no lo encuentra, solicita 
 *   más espacio al sistema.
 * - Los bloques de memoria son gestionados mediante estructuras que contienen metadatos 
 *   como el tamaño del bloque y el puntero al siguiente bloque.
 * - Si un bloque libre encontrado es mayor al tamaño solicitado, se divide en dos bloques.
 * 
 * @note El puntero retornado apunta a la memoria después de los metadatos del bloque.
 * 
 * @example
 * // Ejemplo de uso de la función malloc
 * int* array = (int*)malloc(10 * sizeof(int));
 * if (array != nullptr) {
 *     // Usar la memoria asignada
 *     array[0] = 42;
 * }
 */
void* malloc(size_t size){

    if(size <= 0) return nullptr;

    MemoryBlock* block;

    size = align(sizeof(block) + size);

    if(!heap_start){

        // Inicializa el heap si es la primera llamada a malloc
        block = requestSpace(size);

        if(!block) return nullptr;

        heap_start = block;

    } else {

        // Busca un bloque libre o solicita más memoria
        block = findFreeBlock(size);

        if(!block){

            block = requestSpace(size);

            if(!block) return nullptr;

            // Conecta el nuevo bloque al final de la lista
            MemoryBlock* current = heap_start;

            while(current->next){
                current = current->next;
            }

            current->next = block;
        } else {
            block->isFree = false;
            splitBlock(block, size);
        }


    }

    return (void*)(block + 1); // Devuelve la memoria después del encabezado
}

/**
 * @brief Libera un bloque de memoria previamente asignado.
 *
 * La función `free` marca un bloque de memoria como libre, permitiendo que 
 * pueda ser reutilizado en futuras asignaciones. También intenta combinar 
 * bloques libres adyacentes para reducir la fragmentación de memoria.
 *
 * @param ptr Puntero al bloque de memoria que se desea liberar. Si `ptr` es 
 *            `nullptr`, la función no realiza ninguna operación.
 * 
 * @details
 * - Si el puntero es `nullptr`, la función no realiza ninguna acción.
 * - Calcula la dirección del encabezado del bloque (`MemoryBlock`) restando el 
 *   tamaño del encabezado al puntero recibido.
 * - Marca el bloque correspondiente como libre estableciendo `isFree` en `true`.
 * - Llama a la función `mergeFreeBlocks` para combinar el bloque con bloques 
 *   libres adyacentes.
 * - Llama a `mergeBlocks` para realizar una fusión adicional, si es necesario.
 * 
 * @note Es importante que el puntero pasado a `free` haya sido obtenido 
 *       previamente de una llamada a `malloc`, `calloc` o `realloc`.
 * 
 * @example
 * // Ejemplo de uso de la función free
 * int* array = (int*)malloc(10 * sizeof(int));
 * if (array) {
 *     free(array); // Libera la memoria asignada
 * }
 */
void free(void* ptr){

    if(!ptr) return;

    MemoryBlock* block = (MemoryBlock*)ptr - 1;
    block->isFree = true;

    mergeFreeBlocks(block);
    mergeBlocks(block);
}

/**
 * @brief Cambia el tamaño de un bloque de memoria previamente asignado.
 *
 * Esta función ajusta el tamaño de un bloque de memoria previamente asignado. 
 * Si es necesario, mueve los datos a un nuevo bloque de memoria más grande o 
 * más pequeño. También puede comportarse como `malloc` o `free` dependiendo 
 * de los parámetros de entrada.
 *
 * @param ptr Puntero al bloque de memoria que se desea redimensionar. Si es `nullptr`, 
 *            la función se comporta como `malloc`.
 * @param size Nuevo tamaño en bytes del bloque de memoria. Si es 0, la función 
 *             libera el bloque de memoria y devuelve `nullptr`.
 * 
 * @return Puntero al bloque de memoria redimensionado, o `nullptr` si no se pudo 
 *         asignar memoria.
 * 
 * @details
 * - Si el puntero `ptr` es `nullptr`, se comporta como una llamada a `malloc`.
 * - Si el tamaño solicitado es 0, libera el bloque de memoria utilizando `free` y 
 *   retorna `nullptr`.
 * - Si el tamaño del bloque actual es mayor o igual al nuevo tamaño solicitado, 
 *   simplemente retorna el puntero original.
 * - Si se requiere un bloque más grande, asigna un nuevo bloque de memoria, copia 
 *   los datos existentes al nuevo bloque, libera el bloque anterior y retorna el 
 *   puntero al nuevo bloque.
 * 
 * @note La función puede causar fragmentación de memoria si los bloques liberados 
 *       no se reutilizan adecuadamente.
 * 
 * @example
 * // Ejemplo de uso de la función realloc
 * int* array = (int*)malloc(5 * sizeof(int));
 * if (array) {
 *     array[0] = 42;
 *     array = (int*)realloc(array, 10 * sizeof(int)); // Redimensiona el bloque
 * }
 */
void* realloc(void* ptr, size_t size){

    // Si el puntero ingresado es NULL, se comporta como malloc
    if (!ptr) return malloc(size);

    if (size == 0) {

        // Si el nuevo tamaño es 0, libera el bloque de memoria
        free(ptr);
        return nullptr;
    }

    // Se obtiene el encabezado del bloque actual
    MemoryBlock* block = (MemoryBlock*)ptr - 1;

    // Se verifica si el bloque actual tiene espacio suficiente
    if (block->size >= size) return ptr;

    // Asigna un nuevo bloque de memoria
    void* newPtr = malloc(size);

    if (!newPtr) return nullptr;

    // Copia los datos existentes al nuevo bloque
    memcpy(newPtr, ptr, block->size);
    free(ptr); // Libera el bloque antiguo

    return newPtr;
}

/**
 * @brief Asigna un bloque de memoria inicializado a cero.
 *
 * La función `calloc` asigna un bloque de memoria para almacenar un número 
 * de elementos de un tamaño especificado y asegura que todos los bytes 
 * en el bloque estén inicializados a cero.
 *
 * @param num Número de elementos a asignar.
 * @param size Tamaño en bytes de cada elemento.
 * 
 * @return Puntero al bloque de memoria asignado, o `nullptr` si no se puede 
 *         asignar memoria.
 * 
 * @details
 * - Calcula el tamaño total requerido multiplicando `num` por `size`.
 * - Verifica que no haya un desbordamiento de entero durante la multiplicación.
 * - Utiliza `malloc` para asignar el bloque de memoria.
 * - Inicializa todos los bytes del bloque de memoria a cero utilizando `memset`.
 * 
 * @note Si la multiplicación entre `num` y `size` causa un desbordamiento, la 
 *       función devuelve `nullptr` sin intentar asignar memoria.
 * 
 * @example
 * // Ejemplo de uso de la función calloc
 * int* array = (int*)calloc(10, sizeof(int));
 * if (array) {
 *     // Usar el bloque de memoria asignado
 *     array[0] = 42;
 * }
 */
void* calloc(size_t num, size_t size) {

    // Calcular el tamaño total a asignar
    size_t total_size = num * size;

    // Verificar si hay un desbordamiento en la multiplicación
    if (num != 0 && total_size / num != size) return nullptr;

    // Asignar memoria
    void* ptr = malloc(total_size);
    if (!ptr) return nullptr;

    // Inicializar la memoria asignada a cero
    memset(ptr, 0, total_size);

    return ptr;
}