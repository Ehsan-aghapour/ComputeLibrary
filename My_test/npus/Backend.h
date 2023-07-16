// Define the general backend class structure
template <typename BackendType>
class Backend {
public:
    void initialize();
    void process();
    void cleanup();
};

// Forward declarations for specific backend types
class BackendA;
class BackendB;
