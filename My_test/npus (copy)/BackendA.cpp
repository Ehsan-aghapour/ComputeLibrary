#include "Backend.h"
#include <iostream>

template <>
void Backend<T1>::initialize() {
    std::cerr << "init backend A\n";
    // Implementation for BackendA initialization
}

template <>
void Backend<T1>::process() {
    // Implementation for BackendA processing
}

template <>
void Backend<T1>::cleanup() {
    // Implementation for BackendA cleanup
}
