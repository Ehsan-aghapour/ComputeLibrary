#include <android/trace.h>
#include <fstream>
#include <unistd.h>  // For sleep()

void write_to_trace_marker(const std::string& message) {
    std::ofstream trace_marker("/sys/kernel/debug/tracing/trace_marker");
    if (trace_marker.is_open()) {
        trace_marker << message << std::endl;
        trace_marker.close();
    }
}

int main() {
    // Start custom trace section
    write_to_trace_marker("Simple Test Trace - Start");

    // Start a simple trace section
    ATrace_beginSection("Simple Trace Test");

    // Simulate some work
    sleep(1);

    // End the trace section
    ATrace_endSection();

    // Start custom trace section
    write_to_trace_marker("Simple Test Trace - End");

    return 0;
}
