// Define the general backend class structure
template <typename BackendType>
class Backend {
public:
    void initialize();
    void process();
    void cleanup();
};

// Forward declarations for specific backend types
typedef BackendA T1;
typedef BackendB T2;
