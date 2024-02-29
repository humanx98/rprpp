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

#include "dx_helper.h"

#include "common/HybridProRenderer.h"
#include "common/rprpp_wrappers/Context.h"
#include "common/rprpp_wrappers/Image.h"
#include "common/rprpp_wrappers/PostProcessing.h"

using Microsoft::WRL::ComPtr;

class WithAovsInteropApp {
private:
    int m_width;
    int m_height;
    int m_renderedIterations;
    uint32_t m_framesInFlight;
    DeviceInfo m_deviceInfo;
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
    std::unique_ptr<rprpp::wrappers::Context> m_ppContext;
    std::unique_ptr<rprpp::wrappers::PostProcessing> m_postProcessing;
    std::unique_ptr<rprpp::wrappers::Image> m_dx11output;
    std::unique_ptr<HybridProRenderer> m_hybridproRenderer;
    std::vector<RprPpVkFence> m_fences;
    std::vector<RprPpVkSemaphore> m_frameBuffersReadySemaphores;
    std::vector<RprPpVkSemaphore> m_frameBuffersReleaseSemaphores;
    void initWindow();
    void findAdapter();
    void intiSwapChain();
    void initHybridProAndPostProcessing();
    void resize(int width, int height);
    static void onResize(GLFWwindow* window, int width, int height);
    void mainLoop();

public:
    WithAovsInteropApp(int width, int height, int renderedIterations, uint32_t framesInFlight, Paths paths, DeviceInfo deviceInfo);
    ~WithAovsInteropApp();
    void run();
};