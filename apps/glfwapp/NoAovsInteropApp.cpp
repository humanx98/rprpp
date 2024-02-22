#include "NoAovsInteropApp.h"

#define FORMAT DXGI_FORMAT_B8G8R8A8_UNORM

inline RprPpImageFormat to_rprppformat(DXGI_FORMAT format)
{
    switch (format) {
    case DXGI_FORMAT_R8G8B8A8_UNORM:
        return RPRPP_IMAGE_FROMAT_R8G8B8A8_UNORM;
    case DXGI_FORMAT_B8G8R8A8_UNORM:
        return RPRPP_IMAGE_FROMAT_B8G8R8A8_UNORM;
    default:
        throw std::runtime_error("unsupported image format");
    }
}

NoAovsInteropApp::NoAovsInteropApp(int width, int height, int rendererdIterations, Paths paths, DeviceInfo deviceInfo)
    : m_width(width)
    , m_height(height)
    , m_renderedIterations(rendererdIterations)
    , m_paths(paths)
    , m_deviceInfo(deviceInfo)
    , m_postProcessing(deviceInfo.index)
    , m_hybridproRenderer(deviceInfo.index, std::nullopt, paths.hybridproDll, paths.hybridproCacheDir, paths.assetsDir)
{
    std::cout << "NoAovsInteropApp()" << std::endl;
}

NoAovsInteropApp::~NoAovsInteropApp()
{
    std::cout << "~NoAovsInteropApp()" << std::endl;
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

void NoAovsInteropApp::run()
{
    initWindow();
    findAdapter();
    intiSwapChain();
    resize(m_width, m_height);
    mainLoop();
}

void NoAovsInteropApp::initWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    m_window = glfwCreateWindow(m_width, m_height, "VkDx11 Interop", nullptr, nullptr);
    glfwGetWindowSize(m_window, &m_width, &m_height);
    m_hWnd = glfwGetWin32Window(m_window);
    glfwSetFramebufferSizeCallback(m_window, NoAovsInteropApp::onResize);
    glfwSetWindowUserPointer(m_window, this);
}

void NoAovsInteropApp::findAdapter()
{
    ComPtr<IDXGIFactory4> factory;
    DX_CHECK(CreateDXGIFactory2(0, IID_PPV_ARGS(&factory)));
    ComPtr<IDXGIFactory6> factory6;
    DX_CHECK(factory->QueryInterface(IID_PPV_ARGS(&factory6)));

    LUID* luid = reinterpret_cast<LUID*>(&m_deviceInfo.LUID[0]);
    SUCCEEDED(factory6->EnumAdapterByLuid(*luid, IID_PPV_ARGS(&m_adapter)));
}

void NoAovsInteropApp::intiSwapChain()
{
    DXGI_SWAP_CHAIN_DESC scd = {};
    ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));
    scd.BufferDesc.Width = 0; // use window width
    scd.BufferDesc.Height = 0; // use window height
    scd.BufferDesc.Format = FORMAT;
    scd.SampleDesc.Count = 1;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.BufferCount = 1;
    scd.OutputWindow = m_hWnd;
    scd.Windowed = TRUE;
    auto featureLevel = D3D_FEATURE_LEVEL_11_1;
    DX_CHECK(D3D11CreateDeviceAndSwapChain(
        m_adapter.Get(),
        D3D_DRIVER_TYPE_UNKNOWN,
        nullptr,
        0,
        &featureLevel,
        1,
        D3D11_SDK_VERSION,
        &scd,
        &m_swapChain,
        &m_device,
        nullptr,
        &m_deviceContex));
}

void NoAovsInteropApp::resize(int width, int height)
{
    if (m_width != width || m_height != height || m_backBuffer.Get() == nullptr) {
        m_deviceContex->OMSetRenderTargets(0, nullptr, nullptr);
        m_sharedTextureHandle = nullptr;
        m_sharedTextureResource.Reset();
        m_sharedTexture.Reset();
        m_renderTargetView.Reset();
        m_backBuffer.Reset();
        DX_CHECK(m_swapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0));

        DX_CHECK(m_swapChain->GetBuffer(0, IID_PPV_ARGS(&m_backBuffer)));
        DX_CHECK(m_device->CreateRenderTargetView(m_backBuffer.Get(), nullptr, &m_renderTargetView));
        m_deviceContex->OMSetRenderTargets(1, &m_renderTargetView, nullptr);

        D3D11_TEXTURE2D_DESC sharedTextureDesc = {};
        sharedTextureDesc.Width = width;
        sharedTextureDesc.Height = height;
        sharedTextureDesc.MipLevels = 1;
        sharedTextureDesc.ArraySize = 1;
        sharedTextureDesc.SampleDesc = { 1, 0 };
        sharedTextureDesc.Format = FORMAT;
        sharedTextureDesc.MiscFlags = D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX | D3D11_RESOURCE_MISC_SHARED_NTHANDLE;
        DX_CHECK(m_device->CreateTexture2D(&sharedTextureDesc, nullptr, &m_sharedTexture));
        DX_CHECK(m_sharedTexture->QueryInterface(__uuidof(IDXGIResource1), (void**)&m_sharedTextureResource));
        DX_CHECK(m_sharedTextureResource->CreateSharedHandle(nullptr, GENERIC_ALL, nullptr, &m_sharedTextureHandle));

        D3D11_VIEWPORT viewport;
        ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

        viewport.Width = width;
        viewport.Height = height;
        m_deviceContex->RSSetViewports(1, &viewport);

        m_postProcessing.resize(width, height, to_rprppformat(FORMAT), m_sharedTextureHandle);
        m_hybridproRenderer.resize(width, height);
        float focalLength = m_hybridproRenderer.getFocalLength() / 1000.0f;
        m_postProcessing.setToneMapFocalLength(focalLength);
        m_width = width;
        m_height = height;
    }
}

void NoAovsInteropApp::onResize(GLFWwindow* window, int width, int height)
{
    auto app = static_cast<NoAovsInteropApp*>(glfwGetWindowUserPointer(window));
    app->resize(width, height);
}

void NoAovsInteropApp::copyRprFbToPpStagingBuffer(rpr_aov aov)
{
    size_t size;
    m_hybridproRenderer.getAov(aov, nullptr, 0u, &size);

    StagingBuffer buffer = m_postProcessing.mapStagingBuffer(size);
    m_hybridproRenderer.getAov(aov, buffer.data(), size, nullptr);

    buffer.unmap();
}

void NoAovsInteropApp::mainLoop()
{
    clock_t deltaTime = 0;
    unsigned int frames = 0;
    while (!glfwWindowShouldClose(m_window)) {
        clock_t beginFrame = clock();
        {
            glfwPollEvents();
            for (unsigned int i = 0; i < m_renderedIterations; i++) {
                m_hybridproRenderer.render();
            }
            copyRprFbToPpStagingBuffer(RPR_AOV_COLOR);
            m_postProcessing.copyStagingBufferToAovColor();
            copyRprFbToPpStagingBuffer(RPR_AOV_OPACITY);
            m_postProcessing.copyStagingBufferToAovOpacity();
            copyRprFbToPpStagingBuffer(RPR_AOV_SHADOW_CATCHER);
            m_postProcessing.copyStagingBufferToAovShadowCatcher();
            copyRprFbToPpStagingBuffer(RPR_AOV_REFLECTION_CATCHER);
            m_postProcessing.copyStagingBufferToAovReflectionCatcher();
            copyRprFbToPpStagingBuffer(RPR_AOV_MATTE_PASS);
            m_postProcessing.copyStagingBufferToAovMattePass();
            copyRprFbToPpStagingBuffer(RPR_AOV_BACKGROUND);
            m_postProcessing.copyStagingBufferToAovBackground();
            m_postProcessing.run();
            m_postProcessing.waitQueueIdle();

            IDXGIKeyedMutex* km;
            DX_CHECK(m_sharedTextureResource->QueryInterface(__uuidof(IDXGIKeyedMutex), (void**)&km));
            DX_CHECK(km->AcquireSync(0, INFINITE));
            m_deviceContex->CopyResource(m_backBuffer.Get(), m_sharedTexture.Get());
            DX_CHECK(km->ReleaseSync(0));
            DX_CHECK(m_swapChain->Present(1, 0));
        }
        clock_t endFrame = clock();
        deltaTime += endFrame - beginFrame;
        frames += m_renderedIterations;
        double deltaTimeInSeconds = (deltaTime / (double)CLOCKS_PER_SEC);
        if (deltaTimeInSeconds > 1.0) { // every second
            std::cout << "Iterations per second = "
                      << frames
                      << ", Time per iteration = "
                      << deltaTimeInSeconds * 1000.0 / frames
                      << "ms"
                      << std::endl;
            frames = 0;
            deltaTime -= CLOCKS_PER_SEC;
        }
    }
}