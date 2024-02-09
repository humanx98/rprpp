#pragma once

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <assert.h>
#include <d3d11_3.h>
#include <iostream>
#pragma comment(lib, "d3d11.lib")
#include <dxgi1_6.h>
#pragma comment(lib, "dxgi.lib")
#include <comdef.h>
#include <wrl/client.h>

#include "../common/HybridProRenderer.h"
#include "../common/RprPostProcessing.h"

using Microsoft::WRL::ComPtr;

#define DX_CHECK(f)                                                                                  \
    {                                                                                                \
        HRESULT res = (f);                                                                           \
        if (!SUCCEEDED(res)) {                                                                       \
            printf("[dx11app.h] Fatal : HRESULT is %d in %s at line %d\n", res, __FILE__, __LINE__); \
            _com_error err(res);                                                                     \
            printf("[dx11app.h] messsage : %s\n", err.ErrorMessage());                               \
            assert(false);                                                                           \
        }                                                                                            \
    }

struct GpuIndices {
    int dx11 = 0;
    int vk = 0;
};

struct Paths {
    std::filesystem::path hybridproDll;
    std::filesystem::path hybridproCacheDir;
    std::filesystem::path assetsDir;
};

class Dx11App {
private:
    int m_width;
    int m_height;
    GpuIndices m_gpuIndices;
    Paths m_paths;
    GLFWwindow* m_window = nullptr;
    HWND m_hWnd = nullptr;
    ComPtr<IDXGIAdapter1> m_adapter;
    ComPtr<ID3D11Device> m_device;
    ComPtr<ID3D11Resource> m_backBuffer;
    ComPtr<IDXGISwapChain> m_swapChain;
    ComPtr<ID3D11DeviceContext> m_deviceContex;
    ComPtr<ID3D11RenderTargetView> m_renderTargetView;
    ComPtr<ID3D11Texture2D> m_sharedTexture;
    ComPtr<IDXGIResource1> m_sharedTextureResource;
    HANDLE m_sharedTextureHandle = nullptr;
    RprPostProcessing m_postProcessing;
    HybridProRenderer m_hybridproRenderer;
    void initWindow();
    void findAdapter();
    void intiDx11();
    void copyRprFbToPpStagingBuffer(rpr_aov aov);
    void resize(int width, int height);
    static void onResize(GLFWwindow* window, int width, int height);
    void mainLoop();

public:
    Dx11App(int width, int height, Paths paths, GpuIndices gpuIndices);
    ~Dx11App();
    void run();
};