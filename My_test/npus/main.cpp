#include "Backend.h"

int main() {
    // Create an instance of BackendA
    Backend<BackendA> backendA;
    backendA.initialize();
    backendA.process();
    backendA.cleanup();

    // Create an instance of BackendB
    Backend<BackendB> backendB;
    backendB.initialize();
    backendB.process();
    backendB.cleanup();

    return 0;
}

