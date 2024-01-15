#include <iostream>
#include "dx11app.hpp"
#include "vkcompute.hpp"

int main(int argc, const char* argv[])
{
    std::cout << "VkDx11Interop App started..." << std::endl;

    // Dx11App app(WIDTH, HEIGHT);
    ComputeApplication app;
    try {
        app.run();
    } catch (const std::runtime_error& e) {
        printf("%s\n", e.what());
        return EXIT_FAILURE;
    }
    std::cout << "VkDx11Interop App finished..." << std::endl;
    return 0;
}