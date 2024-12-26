#include <unistd.h>
#include <string.h>
#include <utility>

using word_t = intptr_t;

/**
 * @brief Representa un bloque de memoria en el heap.
 *
 * Cada bloque de memoria consta de una sección de metadatos (`MemoryBlock`) 
 * y una región de datos accesible por el usuario. Los metadatos incluyen:
 *
 * - `size`: Tamaño total del bloque (en bytes) excluyendo los metadatos.
 * - `isFree`: Indica si el bloque está libre (`true`) o ocupado (`false`).
 * - `next`: Puntero al siguiente bloque en la lista enlazada.
 *
 * Layout de la memoria:
 * 
 * +-------------------+
 * | MemoryBlock       |
 * |-------------------|
 * | size              |  // Tamaño del bloque de datos del usuario.
 * |-------------------|
 * | isFree            |  // Indica si el bloque está libre.
 * |-------------------|
 * | next              |  // Puntero al siguiente bloque.
 * +-------------------+
 * | User Memory       |  // Espacio asignado al usuario.
 * +-------------------+
 *
 * Ejemplo:
 * Si se asignan 128 bytes, el bloque incluirá metadatos (`MemoryBlock`) y
 * el espacio para datos del usuario. El puntero devuelto al usuario será 
 * `block + 1`, que apunta directamente a la región de datos.
 */
typedef struct MemoryBlock {
    size_t size;           ///< Tamaño del bloque de datos del usuario.
    bool isFree;           ///< Estado del bloque (`true` si está libre).
    struct MemoryBlock* next; ///< Puntero al siguiente bloque en la lista.
} MemoryBlock;

/**
 * @brief Puntero al inicio del heap.
 *
 * Este puntero apunta al primer bloque en el heap. Se utiliza como punto 
 * de partida para recorrer la lista enlazada de bloques de memoria.
 */
MemoryBlock* heap_start = nullptr;

// Tamaño del encabezado de cada bloque
// const size_t BLOCK_SIZE = sizeof(MemoryBlock);

/**
 * @brief Alinea el tamaño de bytes segun la arquitectura del sistema.
 *
 * @param n Tamaño a alinear.
 * @return Tamaño alineado (múltiplo de 4 u 8 bytes).
 *
 * @details
 * - En sistemas de 32 bits, el tamaño se alinea a 4 bytes.
 * - En sistemas de 64 bits, el tamaño se alinea a 8 bytes.
 * - La alineación garantiza un acceso eficiente a la memoria y cumple con
 *   los requisitos del sistema operativo.
 *
 * Ejemplo:
 * - Para un sistema de 64 bits:
 *   - `align(6)` devuelve `8`.
 *   - `align(9)` devuelve `16`.
 */
static inline size_t align(size_t n) {
    return (n + sizeof(word_t) - 1) & ~(sizeof(word_t) - 1);
}

/**
 * @brief Calcula el tamaño total de la asignación, incluyendo metadatos.
 *
 * @param size Tamaño solicitado por el usuario (en bytes).
 * @return Tamaño total requerido (en bytes), incluyendo el encabezado.
 *
 * @details
 * - Incluye el tamaño de los metadatos (`MemoryBlock`).
 * - Resta una palabra de datos (`word_t`) ya incluida en `MemoryBlock` para evitar duplicación.
 *
 * Ejemplo:
 * - Si el usuario solicita 128 bytes y `sizeof(MemoryBlock) = 24`:
 *   - `allocSize(128)` devuelve `128 + 24 - 8 = 144 bytes` (en un sistema de 64 bits).
 */
static inline size_t allocSize(size_t size) {
    return size + sizeof(MemoryBlock) - sizeof(std::declval<MemoryBlock>().size);
}

/**
 * @brief Implementa el algoritmo First-Fit para encontrar bloques libres.
 *
 * @param size Tamaño mínimo requerido (en bytes).
 * @return Puntero al primer bloque libre que cumpla con el tamaño, o `nullptr` si no se encuentra.
 *
 * @details
 * - Recorre la lista enlazada de bloques de memoria comenzando desde `heap_start`.
 * - Devuelve el primer bloque que esté libre y cuyo tamaño sea suficiente.
 * - Es un algoritmo con complejidad `O(n)` en el peor de los casos.
 *
 * @note
 * - Si no se encuentra un bloque adecuado, se devuelve `nullptr`.
 * - Este algoritmo puede ser ineficiente si hay muchos bloques pequeños fragmentados.
 */
MemoryBlock* firstFit(size_t size) {
    auto block = heap_start;

    while (block != nullptr) {
        if (block->isFree && block->size >= size) {
            return block; // Bloque adecuado encontrado.
        }
        block = block->next; // Avanzar al siguiente bloque.
    }

    return nullptr; // No se encontró un bloque libre adecuado.
}

/**
 * @brief Encuentra un bloque de memoria libre utilizando un algoritmo de búsqueda.
 *
 * @param size Tamaño mínimo requerido (en bytes).
 * @return Puntero al bloque encontrado, o `nullptr` si no se encuentra.
 *
 * @details
 * - Actualmente utiliza el algoritmo First-Fit para encontrar un bloque.
 * - Sirve como interfaz para implementar otros algoritmos de búsqueda 
 *   (Best-Fit, Next-Fit, etc.) en el futuro.
 *
 * @example
 * MemoryBlock* block = findBlock(128);
 * if (block) {
 *     std::cout << "Bloque encontrado con tamaño: " << block->size << " bytes.\n";
 * } else {
 *     std::cout << "No se encontró un bloque adecuado.\n";
 * }
 */
MemoryBlock *findBlock(size_t size) {
    return firstFit(size);
}