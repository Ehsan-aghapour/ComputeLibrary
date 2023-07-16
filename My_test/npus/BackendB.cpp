#include "Backend.h"
#include <iostream>

template <>
void Backend<BackendB>::initialize() {
    std::cerr<<"B\n";
    // Implementation for BackendB initialization
}

template <>
void Backend<BackendB>::process() {
    // Implementation for BackendB processing
}

template <>
void Backend<BackendB>::cleanup() {
    // Implementation for BackendB cleanup
}
