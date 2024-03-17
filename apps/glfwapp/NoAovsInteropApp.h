#pragma once

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <d3d11_3.h>
#pragma comment(lib, "d3d11.lib")
#include <dxgi1_6.h>
#pragma comment(lib, "dxgi.lib")
#include <comdef.h>
#include <wrl/client.h>

#include "dx_helper.h"

#include "common/HybridProRenderer.h"
#include "common/rprpp_wrappers/Buffer.h"
#include "common/rprpp_wrappers/Context.h"
#include "common/rprpp_wrappers/Image.h"
#include "common/rprpp_wrappers/filters/BloomFilter.h"
#include "common/rprpp_wrappers/filters/ComposeColorShadowReflectionFilter.h"
#include "common/rprpp_wrappers/filters/ComposeOpacityShadowFilter.h"
#include "common/rprpp_wrappers/filters/DenoiserFilter.h"
#include "common/rprpp_wrappers/filters/ToneMapFilter.h"

using Microsoft::WRL::ComPtr;

class NoAovsInteropApp {
private:
    int m_width;
    int m_height;
    int m_renderedIterations;
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
    std::unique_ptr<rprpp::wrappers::filters::BloomFilter> m_bloomFilter;
    std::unique_ptr<rprpp::wrappers::filters::ComposeColorShadowReflectionFilter> m_composeColorShadowReflectionFilter;
    std::unique_ptr<rprpp::wrappers::filters::ComposeOpacityShadowFilter> m_composeOpacityShadowFilter;
    std::unique_ptr<rprpp::wrappers::filters::DenoiserFilter> m_denoiserFilter;
    std::unique_ptr<rprpp::wrappers::filters::ToneMapFilter> m_tonemapFilter;
    std::unique_ptr<rprpp::wrappers::Buffer> m_buffer;
    std::unique_ptr<rprpp::wrappers::Image> m_dx11output;
    std::unique_ptr<rprpp::wrappers::Image> m_output;
    std::unique_ptr<rprpp::wrappers::Image> m_aovColor;
    std::unique_ptr<rprpp::wrappers::Image> m_aovOpacity;
    std::unique_ptr<rprpp::wrappers::Image> m_aovShadowCatcher;
    std::unique_ptr<rprpp::wrappers::Image> m_aovReflectionCatcher;
    std::unique_ptr<rprpp::wrappers::Image> m_aovMattePass;
    std::unique_ptr<rprpp::wrappers::Image> m_aovBackground;
    HybridProRenderer m_hybridproRenderer;
    void initWindow();
    void findAdapter();
    void intiSwapChain();
    void copyRprFbToPpStagingBuffer(rpr_aov aov);
    void resize(int width, int height);
    static void onResize(GLFWwindow* window, int width, int height);
    void mainLoop();

public:
    NoAovsInteropApp(int width, int height, int rendererdIterations, Paths paths, DeviceInfo deviceInfo);
    ~NoAovsInteropApp();
    void run();
};