#include "Backend.h"
#include <iostream>

template <>
void Backend<BackendA>::initialize() {
    std::cerr << "init backend A\n";
    // Implementation for BackendA initialization
}

template <>
void Backend<BackendA>::process() {
    // Implementation for BackendA processing
}

template <>
void Backend<BackendA>::cleanup() {
    // Implementation for BackendA cleanup
}
