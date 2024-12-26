#include "memoryBlock.h"

/**
 * @brief Divide un bloque de memoria en dos bloques más pequeños.
 *
 * Esta función se utiliza para optimizar el uso de memoria dividiendo un bloque 
 * grande en dos partes más pequeñas. El primer bloque se ajusta al tamaño solicitado, 
 * mientras que el espacio sobrante se convierte en un nuevo bloque libre.
 * 
 * Layout antes de la división:
 * +-------------------+----------------------+
 * |    Metadata       |      User Memory     |
 * |   (MemoryBlock)   |     (block->size)    |
 * +-------------------+----------------------+

 * Layout después de la división:
 * +-------------------+---------------------+-------------------+-------------------+
 * |     Metadata      |   User Memory (A)   |     Metadata      |  User Memory (B)  |
 * |   (MemoryBlock)   |      (size)         |   (MemoryBlock)   | (newBlock->size)  |
 * +-------------------+---------------------+-------------------+-------------------+
 * |  <--- block --->  | <-- block->next --> | <-- newBlock -->  | <-- Rest of Mem ->|
 * +-------------------+---------------------+-------------------+-------------------+
 *
 * @param block Puntero al bloque de memoria que se desea dividir.
 * @param size Tamaño solicitado para el primer bloque después de la división.
 *
 * @details
 * - La función verifica primero si el bloque es lo suficientemente grande para ser dividido.
 * - Si es posible dividir el bloque, se crea un nuevo bloque en el espacio sobrante.
 * - El nuevo bloque se inicializa como libre y se enlaza en la lista de bloques.
 * - El bloque original se ajusta al tamaño solicitado y se actualiza su puntero al siguiente bloque.
 *
 * @note 
 * Esta función no realiza ninguna operación si el bloque no tiene suficiente espacio para 
 * acomodar el tamaño solicitado más el espacio necesario para el encabezado del nuevo bloque 
 * de memoria.
 *
 * @example
 * // Dado un bloque de memoria grande, divídelo en dos partes:
 * MemoryBlock* block = findFreeBlock(128);
 * splitBlock(block, 64);
 */
void splitBlock(MemoryBlock* block, size_t size) {

    if (block->size <= size + sizeof(MemoryBlock)) return;

    // Crear un nuevo bloque dentro del espacio sobrante.
    MemoryBlock* newBlock = (MemoryBlock*)((char*)(block + 1) + size);

    // Inicializar el nuevo bloque.
    newBlock->size = block->size - size - sizeof(MemoryBlock);
    newBlock->isFree = true;
    newBlock->next = block->next;
    block->size = size;
    block->next = newBlock;
}

/**
 * @brief Combina bloques de memoria libres adyacentes para reducir la fragmentación.
 *
 * Esta función optimiza el uso de memoria al unir bloques de memoria contiguos que 
 * están marcados como libres. Esto reduce la fragmentación del heap y mejora la 
 * disponibilidad de bloques más grandes para futuras asignaciones.
 * 
 * Inicialmente:
 * +-------------------+-------------------+-------------------+----------------------+
 * |    Metadata A     |   User Memory A   |     Metadata B    |     User Memory B    |
 * |   (MemoryBlock)   |   (block->size)   |   (MemoryBlock)   |  (block->next->size) |
 * +-------------------+-------------------+-------------------+----------------------+
 *
 * Después de combinar A y B:
 * +-------------------+---------------------------------------+-------------------+
 * |    Metadata A     |             User Memory A+B           |    Metadata C     |
 * |   (MemoryBlock)   |      (block->size += next->size)      |   (MemoryBlock)   |
 * +-------------------+---------------------------------------+-------------------+
 *
 * Continúa con el siguiente bloque (C) si también está libre.
 *
 * @param block Puntero al bloque de memoria desde donde se inicia la combinación.
 *              Debe estar marcado como libre para que la combinación sea válida.
 *
 * @details
 * - La función primero verifica que el bloque proporcionado no sea nulo y esté marcado como libre.
 * - Luego, recorre los bloques adyacentes para detectar si están libres.
 * - Si encuentra bloques libres adyacentes, combina sus tamaños y ajusta los punteros de la lista.
 * - El proceso continúa hasta que no haya más bloques libres adyacentes.
 *
 * @note 
 * - Esta función es útil después de liberar memoria (`free`) para consolidar bloques libres.
 * - No realiza ninguna operación si el bloque proporcionado está en uso o es nulo.
 *
 * @example
 * // Combina bloques libres en una lista de memoria:
 * MemoryBlock* block = findFreeBlock(64);
 * mergeBlocks(block);
 */
void mergeBlocks(MemoryBlock* block) {

    if (!block || !block->isFree) return;

    // Combinar con el siguiente bloque si está libre.
    while (block->next && block->next->isFree) {
        block->size += sizeof(MemoryBlock) + block->next->size;
        block->next = block->next->next;
    }
}

/**
 * @brief Combina bloques de memoria libres adyacentes para reducir la fragmentación.
 *
 * Esta función intenta fusionar el bloque dado con bloques libres adyacentes 
 * en el heap. Primero verifica si el siguiente bloque es libre y lo combina 
 * con el bloque actual si es posible. Luego, busca el bloque previo al bloque 
 * actual para intentar fusionarlo si también es libre. Este proceso ayuda a 
 * reducir la fragmentación de memoria al unir bloques pequeños en uno más grande.
 * 
 * Inicialmente:
 * +-------------------+-------------------+-------------------+---------------------+-------------------+---------------------------+
 * |    Metadata A     |   User Memory A   |    Metadata B     |   User Memory B     |    Metadata C     |       User Memory C       |
 * |   (MemoryBlock)   |   (block->size)   |   (MemoryBlock)   | (block->next->size) |   (MemoryBlock)   | (block->next->next->size) |
 * +-------------------+-------------------+-------------------+---------------------+-------------------+---------------------------+
 *
 * Paso 1: Combinar el bloque actual (A) con el siguiente (B) si es libre:
 * +-------------------+---------------------------------------+-------------------+----------------------+
 * |    Metadata A     |           User Memory A+B             |     Metadata C    |     User Memory C    |
 * |   (MemoryBlock)   |     (block->size += next->size)       |   (MemoryBlock)   |  (block->next->size) |
 * +-------------------+---------------------------------------+-------------------+----------------------+
 *
 * Paso 2: Combinar el bloque previo (si existe y es libre) con el bloque actual:
 * +---------------------------------------+----------------------------------------+-------------------+
 * |          Metadata Prev (A')           |           User Memory Prev+A+B         |    Metadata C     |
 * |     (block->size += prev->size)       |  (current->size += sizeof(MemoryBlock) |   (MemoryBlock)   |
 * +---------------------------------------+----------------------------------------+-------------------+
 *
 * Resultado final (si ambos pasos fueron exitosos):
 * +---------------------------------------+------------------------------------------+
 * |            Metadata Prev (A')         |           User Memory Prev+A+B+C         |
 * |      (block->size += prev->size)      |   (current->size += C->size + sizeof())  |
 * +---------------------------------------+------------------------------------------+
 *
 * Nota:
 * - `Metadata` representa la estructura `MemoryBlock`, que contiene información como punteros y tamaño.
 * - `User Memory` es el espacio asignado al usuario dentro del bloque.
 * - El bloque `C` se combina solo si también está marcado como libre.
 *
 * @param block Puntero al bloque de memoria que se desea combinar con bloques libres adyacentes.
 *
 * @details
 * - La función modifica la estructura de los bloques en el heap al actualizar
 *   sus tamaños y los punteros al siguiente bloque.
 * - No realiza ninguna operación si no hay bloques adyacentes libres.
 *
 * @note 
 * Es importante que el bloque pasado a esta función ya esté marcado como libre.
 * 
 * @example 
 * Dado un bloque marcado como libre, intenta combinarlo con sus vecinos.
 * mergeFreeBlocks(block);
 */
void mergeFreeBlocks(MemoryBlock* block){

    // Intentamos combinar con el siguiente bloque, si es libre.
    if (block->next && block->next->isFree) {
        block->size += sizeof(MemoryBlock) + block->next->size;
        block->next = block->next->next;
    }

    // Ahora, intentamos combinar con el bloque previo.
    MemoryBlock* current = heap_start;
    while (current) {
        if (current->next == block && current->isFree) {
            current->size += sizeof(MemoryBlock) + block->size;
            current->next = block->next;
            break;
        }
        current = current->next;
    }
}

/**
 * @brief Busca un bloque de memoria libre en el heap que cumpla con el tamaño requerido.
 *
 * Esta función recorre la lista de bloques de memoria en el heap para encontrar
 * un bloque que esté libre y tenga un tamaño igual o mayor al solicitado.
 *
 * @param size Tamaño mínimo requerido para el bloque de memoria (en bytes).
 * @return Retorna un puntero al bloque de memoria que cumple con los requisitos, o `nullptr` si no se encuentra.
 *
 * @details
 * - La función comienza desde el inicio del heap (`heap_start`).
 * - Recorre secuencialmente cada bloque de memoria hasta encontrar uno que:
 * - Esté marcado como libre (`isFree == true`).
 * - Tenga un tamaño mayor o igual al solicitado.
 * - Si encuentra un bloque adecuado, devuelve un puntero a ese bloque.
 * - Si no se encuentra ningún bloque que cumpla los criterios, devuelve `nullptr`.
 *
 * @note 
 * - La función no modifica el estado de los bloques, solo realiza una búsqueda.
 * - Es útil como parte del proceso de asignación de memoria (`malloc`), ya que
 *   evita la necesidad de solicitar espacio adicional al sistema operativo si
 *   existe un bloque adecuado disponible en el heap.
 *
 * @example
 * // Intentar encontrar un bloque libre con al menos 128 bytes:
 * MemoryBlock* block = findFreeBlock(128);
 * if (block) {
 *     std::cout << "Bloque encontrado con tamaño: " << block->size << " bytes\n";
 * } else {
 *     std::cout << "No se encontró ningún bloque libre.\n";
 * }
 */
MemoryBlock* findFreeBlock(size_t size){

    MemoryBlock* current = heap_start;

    while(current){
        if(current->isFree && current->size >= size){
            return current;
        }
        current = current->next;
    }

    return nullptr;
}

/**
 * @brief Solicita un bloque de memoria al sistema operativo para ampliación del heap.
 *
 * Esta función utiliza `sbrk` para solicitar al sistema operativo un espacio adicional 
 * de memoria en el heap y lo configura como un nuevo bloque de memoria.
 *
 * @param size Tamaño del bloque de memoria a solicitar (en bytes).
 * @return Retorna un puntero al nuevo bloque de memoria asignado, o `nullptr` si la solicitud falla.
 *
 * @details
 * - `sbrk(0)` se utiliza para obtener la dirección actual del final del heap.
 * - `sbrk(allocSize(size))` incrementa el tamaño del heap en la cantidad especificada
 *   por `allocSize(size)` (incluyendo espacio adicional para el encabezado del bloque).
 * - Si `sbrk` devuelve `(void*) -1`, significa que la solicitud de memoria falló.
 * - Si `heap_start` es `nullptr`, el bloque solicitado se establece como el inicio del heap.
 * - El bloque solicitado se inicializa con el tamaño solicitado, se marca como no libre (`isFree = false`),
 *   y su puntero `next` se establece en `nullptr`.
 *
 * @note 
 * - `allocSize(size)` debe calcular el tamaño total necesario, incluyendo metadatos
 *   como el encabezado del bloque.
 * - Es importante manejar el caso en que `sbrk` falla para evitar comportamientos indefinidos.
 * - Esta función se utiliza principalmente cuando no hay bloques libres disponibles
 *   en el heap y se requiere espacio adicional.
 *
 * @example
 * // Solicitar un bloque de memoria de 256 bytes:
 * MemoryBlock* newBlock = requestSpace(256);
 * if (newBlock) {
 *     std::cout << "Nuevo bloque asignado con tamaño: " << newBlock->size << " bytes\n";
 * } else {
 *     std::cout << "Fallo al solicitar memoria al sistema operativo.\n";
 * }
 */
MemoryBlock* requestSpace(size_t size){

    // Obtener la dirección actual del final del heap.
    MemoryBlock* block = (MemoryBlock*) sbrk(0);

    // Incrementar el tamaño del heap en la cantidad necesaria.
    if(sbrk(allocSize(size)) == (void*) - 1) return nullptr;

    // Si el heap no ha sido inicializado, este bloque será el inicio del heap
    if(heap_start == nullptr) heap_start = block;

    // Inicializa el nuevo bloque de memoria
    block->size = size;
    block->isFree = false;
    block->next = nullptr;

    return block;
}