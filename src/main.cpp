#include <iostream>
#include "malloc.cpp"

using string = std::string;

// ------ MEMORY ALLOCATOR IN C++ -------

// Función meminfo: Muestra estadísticas de uso de memoria
void memoryInfo() {
    size_t totalMemory = 0;     // Memoria total asignada
    size_t freeMemory = 0;      // Memoria libre
    size_t usedMemory = 0;      // Memoria en uso
    size_t blockCount = 0;      // Número total de bloques
    size_t freeBlockCount = 0;  // Número de bloques libres

    MemoryBlock* current = heap_start;

    while (current) {
        blockCount++;
        totalMemory += current->size;

        if (current->isFree) {
            freeBlockCount++;
            freeMemory += current->size;
        } else {
            usedMemory += current->size;
        }

        current = current->next;
    }

    // Imprimir estadísticas
    std::cout << "+---------------------------------------+" << std::endl;
    std::cout << "Memory Allocator Statistics:" << std::endl;
    std::cout << "Total Memory: " << totalMemory << " bytes" << std::endl;
    std::cout << "Used Memory: " << usedMemory << " bytes" << std::endl;
    std::cout << "Free Memory: " << freeMemory << " bytes" << std::endl;
    std::cout << "Total Blocks: " << blockCount << std::endl;
    std::cout << "Free Blocks: " << freeBlockCount << std::endl;
}

int main(){

    pid_t processID = getpid();
    std::cout << "El ID del proceso es: " << processID << std::endl;

    std::cin.get();

    // Reservar memoria para un entero
    int* ptr = (int*)malloc(sizeof(int));
    *ptr = 201;
    std::cout << "Valor almacenado: " << *ptr << std::endl;
    std::cout << "Cantidad Almacenada: " << sizeof(*ptr) << " bytes" << std::endl;

    std::cin.get();

    bool* ptr2 = (bool*)malloc(sizeof(bool));
    *ptr2 = true;
    std::cout << "Valor almacenado: " << *ptr2 << std::endl;
    std::cout << "Cantidad Almacenada: " << sizeof(*ptr2) << " bytes" << std::endl;

    int* ptr3 = (int*)malloc(8);

    std::cin.get();

    // Cambiar el tamaño del bloque
    ptr = (int*)realloc(ptr, 2 * sizeof(int));
    ptr[1] = 84;
    std::cout << "Nuevos valores: " << ptr[0] << ", " << ptr[1] << std::endl;

    memoryInfo();
    std::cin.get();

    // Liberar memoria
    free(ptr);

    memoryInfo();
    std::cin.get();

    // Usar calloc
    int* arr = (int*)calloc(10, sizeof(int));
    for (int i = 0; i < 5; ++i) {
        std::cout << "arr[" << i << "] = " << arr[i] << std::endl;
    }

    memoryInfo();
    std::cin.get();

    free(arr);

    memoryInfo();
    std::cin.get();
}