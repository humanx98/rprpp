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

#include "common.hpp"
#include "vkcompute.hpp"

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
    int width;
    int height;
    GpuIndices gpuIndices;
    GLFWwindow* window = nullptr;
    HWND hWnd = nullptr;
    ComPtr<IDXGIAdapter1> adapter;
    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11Resource> backBuffer;
    ComPtr<IDXGISwapChain> swapChain;
    ComPtr<ID3D11DeviceContext> deviceContex;
    ComPtr<ID3D11RenderTargetView> renderTargetView;
    ComPtr<ID3D11Texture2D> sharedTexture;
    ComPtr<IDXGIResource1> sharedTextureResource;
    HANDLE sharedTextureHandle = nullptr;
    VkCompute vkcompute;

public:
    Dx11App(int w, int h, GpuIndices gi)
        : width(w)
        , height(h)
        , gpuIndices(gi)
    {
    }

    ~Dx11App()
    {
        vkcompute.cleanup();
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void run()
    {
        initWindow();
        findAdapter();
        intiDx11();
        vkcompute.init(sharedTextureHandle, width, height, gpuIndices);
        mainLoop();
    }

    void initWindow()
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        window = glfwCreateWindow(width, height, "VkDx11 Interop", nullptr, nullptr);
        hWnd = glfwGetWin32Window(window);
    }

    void findAdapter()
    {
        ComPtr<IDXGIFactory4> factory;
        DX_CHECK(CreateDXGIFactory2(0, IID_PPV_ARGS(&factory)));
        ComPtr<IDXGIFactory6> factory6;
        DX_CHECK(factory->QueryInterface(IID_PPV_ARGS(&factory6)));

        std::cout << "[dx11app.hpp] "
                  << "All Adapters:" << std::endl;

        ComPtr<IDXGIAdapter1> tmpAdapter;
        DXGI_ADAPTER_DESC selectedAdapterDesc;
        int adapterCount = 0;
        for (UINT adapterIndex = 0; SUCCEEDED(factory6->EnumAdapterByGpuPreference(adapterIndex, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&tmpAdapter))); ++adapterIndex) {
            adapterCount++;

            DXGI_ADAPTER_DESC desc;
            tmpAdapter->GetDesc(&desc);

            std::wcout << "[dx11app.hpp] "
                       << "\t" << adapterIndex << ". " << desc.Description << std::endl;

            if (adapterIndex == gpuIndices.dx11) {
                adapter = tmpAdapter;
                selectedAdapterDesc = desc;
            }
        }

        if (adapterCount <= gpuIndices.dx11) {
            throw std::runtime_error("[dx11app.hpp] could not find a IDXGIAdapter1, gpuIndices.dx11 is out of range");
        }

        std::wcout << "[dx11app.hpp] "
                   << "Selected adapter: " << selectedAdapterDesc.Description << std::endl;
    }

    void intiDx11()
    {
        auto format = DXGI_FORMAT_R8G8B8A8_UNORM;
        DXGI_SWAP_CHAIN_DESC scd = {};
        ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));
        scd.BufferDesc.Width = width;
        scd.BufferDesc.Height = height;
        scd.BufferDesc.Format = format;
        scd.SampleDesc.Count = 1;
        scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        scd.BufferCount = 1;
        scd.OutputWindow = hWnd;
        scd.Windowed = TRUE;
        auto featureLevel = D3D_FEATURE_LEVEL_11_1;
        DX_CHECK(D3D11CreateDeviceAndSwapChain(
            adapter.Get(),
            D3D_DRIVER_TYPE_UNKNOWN,
            nullptr,
            0,
            &featureLevel,
            1,
            D3D11_SDK_VERSION,
            &scd,
            &swapChain,
            &device,
            nullptr,
            &deviceContex));
        DX_CHECK(swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer)));
        DX_CHECK(device->CreateRenderTargetView(backBuffer.Get(), nullptr, &renderTargetView));
        deviceContex->OMSetRenderTargets(1, &renderTargetView, nullptr);

        D3D11_TEXTURE2D_DESC sharedTextureDesc = {};
        sharedTextureDesc.Width = width;
        sharedTextureDesc.Height = height;
        sharedTextureDesc.MipLevels = 1;
        sharedTextureDesc.ArraySize = 1;
        sharedTextureDesc.SampleDesc = { 1, 0 };
        sharedTextureDesc.Format = format;
        sharedTextureDesc.MiscFlags = D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX | D3D11_RESOURCE_MISC_SHARED_NTHANDLE;
        DX_CHECK(device->CreateTexture2D(&sharedTextureDesc, nullptr, &sharedTexture));
        DX_CHECK(sharedTexture->QueryInterface(__uuidof(IDXGIResource1), (void**)&sharedTextureResource));
        DX_CHECK(sharedTextureResource->CreateSharedHandle(nullptr, GENERIC_ALL, nullptr, &sharedTextureHandle));

        D3D11_VIEWPORT viewport;
        ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

        viewport.Width = width;
        viewport.Height = height;
        deviceContex->RSSetViewports(1, &viewport);
    }

    void mainLoop()
    {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();

            vkcompute.render();

            IDXGIKeyedMutex* km;
            DX_CHECK(sharedTextureResource->QueryInterface(__uuidof(IDXGIKeyedMutex), (void**)&km));
            DX_CHECK(km->AcquireSync(0, INFINITE));
            deviceContex->CopyResource(backBuffer.Get(), sharedTexture.Get());
            DX_CHECK(km->ReleaseSync(0));

            DX_CHECK(swapChain->Present(1, 0));
        }
    }
};