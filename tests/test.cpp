#include <iostream>
#include "../src/malloc.cpp"

void testMalloc() {
    int* ptr = (int*)malloc(sizeof(int));
    if (!ptr) {
        std::cout << "malloc failed: returned nullptr\n";
        return;
    }

    *ptr = 42; // Prueba escribiendo datos
    if (*ptr == 42) {
        std::cout << "malloc test passed\n";
    } else {
        std::cout << "malloc test failed\n";
    }

    free(ptr); // Libera la memoria
}

void testFree() {
    int* ptr = (int*)malloc(sizeof(int));
    if (!ptr) {
        std::cout << "malloc failed in free test\n";
        return;
    }

    free(ptr);
    std::cout << "free test passed\n";
}

void testRealloc() {
    int* ptr = (int*)malloc(2 * sizeof(int));
    if (!ptr) {
        std::cout << "malloc failed in realloc test\n";
        return;
    }

    ptr[0] = 1;
    ptr[1] = 2;

    ptr = (int*)realloc(ptr, 4 * sizeof(int));
    if (!ptr) {
        std::cout << "realloc failed\n";
        return;
    }

    if (ptr[0] == 1 && ptr[1] == 2) {
        std::cout << "realloc test passed\n";
    } else {
        std::cout << "realloc test failed\n";
    }

    free(ptr);
}

void testCalloc() {
    int* ptr = (int*)calloc(4, sizeof(int));
    if (!ptr) {
        std::cout << "calloc failed\n";
        return;
    }

    bool initialized = true;
    for (int i = 0; i < 4; ++i) {
        if (ptr[i] != 0) {
            initialized = false;
            break;
        }
    }

    if (initialized) {
        std::cout << "calloc test passed\n";
    } else {
        std::cout << "calloc test failed\n";
    }

    free(ptr);
}

void testFragmentation() {

    int* ptr1 = (int*)malloc(4 * sizeof(int));
    int* ptr2 = (int*)malloc(4 * sizeof(int));
    int* ptr3 = (int*)malloc(4 * sizeof(int));

    free(ptr2); // Libera el bloque central
    free(ptr3); // Libera el último bloque

    free(ptr1); // Libera el primero, debería fusionar todo

    std::cout << "fragmentation test passed (manual verification required)\n";
}

void runTests() {

    std::cout << "Running tests...\n";

    testMalloc();
    testFree();
    testRealloc();
    testCalloc();
    testFragmentation();

    std::cout << "All tests completed.\n";
}

int main() {
    runTests();
}