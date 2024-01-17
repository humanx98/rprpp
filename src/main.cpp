#include "dx11app.hpp"
#include <iostream>

int main(int argc, const char* argv[])
{
    std::cout << "VkDx11Interop App started..." << std::endl;

    // notice that resolution is hardcoded in the vk compute shader
    Dx11App app(1600, 1000, RequestHighPerformanceDevice { .forDx11 = true, .forVk = true });
    try {
        app.run();
    } catch (const std::runtime_error& e) {
        printf("%s\n", e.what());
        return EXIT_FAILURE;
    }
    std::cout << "VkDx11Interop App finished..." << std::endl;
    return 0;
}