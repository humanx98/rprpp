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

#include "HybridProRenderer.hpp"
#include <PostProcessing.hpp>
#include "common.hpp"

using Microsoft::WRL::ComPtr;

#define DX_CHECK(f)                                                                                    \
    {                                                                                                  \
        HRESULT res = (f);                                                                             \
        if (!SUCCEEDED(res)) {                                                                         \
            printf("[dx11app.hpp] Fatal : HRESULT is %d in %s at line %d\n", res, __FILE__, __LINE__); \
            _com_error err(res);                                                                       \
            printf("[dx11app.hpp] messsage : %s\n", err.ErrorMessage());                               \
            assert(false);                                                                             \
        }                                                                                              \
    }

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
    std::optional<PostProcessing> m_postProcessing;
    std::unique_ptr<HybridProRenderer> m_hybridproRenderer;

public:
    Dx11App(int width, int height, Paths paths, GpuIndices gpuIndices);
    ~Dx11App();
    void run();
    void initWindow();
    void findAdapter();
    void intiDx11();
    void resize(int width, int height);
    static void onResize(GLFWwindow* window, int width, int height);
    void mainLoop();
};