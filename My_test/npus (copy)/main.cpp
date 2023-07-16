#include "Backend.h"

int main() {
    // Create an instance of BackendA
    Backend<T1> backendA;
    backendA.initialize();
    backendA.process();
    backendA.cleanup();

    // Create an instance of BackendB
    Backend<T2> backendB;
    backendB.initialize();
    backendB.process();
    backendB.cleanup();

    return 0;
}

