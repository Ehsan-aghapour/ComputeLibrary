#include "Backend.h"
#include <iostream>

template <>
void Backend<T2>::initialize() {
    std::cerr<<"B\n";
    // Implementation for BackendB initialization
}

template <>
void Backend<T2>::process() {
    // Implementation for BackendB processing
}

template <>
void Backend<T2>::cleanup() {
    // Implementation for BackendB cleanup
}
