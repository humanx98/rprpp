#pragma once

#define GLFW_EXPOSE_NATIVE_WIN32
#include <iostream>
#include <assert.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <d3d11_3.h>
#pragma comment(lib, "d3d11.lib")
#include <dxgi.h>
#pragma comment(lib, "dxgi.lib")

#define DX_CHECK(f)                                                                \
    {                                                                                     \
        HRESULT res = (f);                                                               \
        if (!SUCCEEDED(res)) {                                                          \
            printf("Fatal : HRESULT is %d in %s at line %d\n", res, __FILE__, __LINE__); \
            assert(false);                                                    \
        }                                                                                 \
    }

class Dx11App {
private:
    int width;
    int height;
    GLFWwindow* window;
    HWND hWnd;
    ID3D11Device* device;
    ID3D11Resource* backBuffer;
	IDXGISwapChain* swapChain;
	ID3D11DeviceContext* deviceContex;
	ID3D11RenderTargetView* renderTargetView;
    
    ID3D11Texture2D *sharedTexture;
    IDXGIResource1 *sharedTextureResource;
    HANDLE sharedTextureHandle;

public:
    Dx11App(int w, int h) : width(w), height(h)
    {
    }

    ~Dx11App()
    {
        backBuffer->Release();
        sharedTexture->Release();
        renderTargetView->Release();
        deviceContex->Release();
        swapChain->Release();
        device->Release();

        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void run() 
    {
        initWindow();
        initDx11();
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

    void initDx11()
    {
        DXGI_SWAP_CHAIN_DESC scd = {};
        ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));
        scd.BufferDesc.Width = width;
        scd.BufferDesc.Height = height;
        scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        scd.SampleDesc.Count = 1;
        scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        scd.BufferCount = 1;
        scd.OutputWindow = hWnd;
        scd.Windowed = TRUE;

        auto featureLevel = D3D_FEATURE_LEVEL_11_1;
        DX_CHECK(D3D11CreateDeviceAndSwapChain(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            0,
            &featureLevel,
            1,
            D3D11_SDK_VERSION,
            &scd,
            &swapChain,
            &device,
            nullptr,
            &deviceContex
        ));
        
        DX_CHECK(swapChain->GetBuffer(0, __uuidof(ID3D11Resource), reinterpret_cast<void**>(&backBuffer)));
        DX_CHECK(device->CreateRenderTargetView(backBuffer, nullptr, &renderTargetView));
        deviceContex->OMSetRenderTargets(1, &renderTargetView, nullptr);

        D3D11_TEXTURE2D_DESC sharedTextureDesc = {};
        sharedTextureDesc.Width = width;
        sharedTextureDesc.Height = height;
        sharedTextureDesc.MipLevels = 1;
        sharedTextureDesc.ArraySize = 1;
        sharedTextureDesc.SampleDesc = {1, 0};
        sharedTextureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sharedTextureDesc.MiscFlags = D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX | D3D11_RESOURCE_MISC_SHARED_NTHANDLE;
        DX_CHECK(device->CreateTexture2D(&sharedTextureDesc, nullptr, &sharedTexture));
        DX_CHECK(sharedTexture->QueryInterface(__uuidof(IDXGIResource1), (void **)&sharedTextureResource));
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
            // DX_CHECK(swapChain->Present(1u, 0u));
            // float color[4] = {
            //     0.3f, 0.5f, 0.8f, 1.0f
            // };
            // deviceContex->ClearRenderTargetView(renderTargetView, color);
            
            IDXGIKeyedMutex *km;
            DX_CHECK(sharedTextureResource->QueryInterface(__uuidof(IDXGIKeyedMutex), (void **)&km));

            DX_CHECK(km->AcquireSync(0, INFINITE));
            deviceContex->CopyResource(backBuffer, sharedTexture);
            DX_CHECK(km->ReleaseSync(0));
            DX_CHECK(swapChain->Present(1, 0));
        }
    }
};