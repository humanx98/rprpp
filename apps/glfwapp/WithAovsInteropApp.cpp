#include "WithAovsInteropApp.h"

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

WithAovsInteropApp::WithAovsInteropApp(int width, int height, int renderedIterations, uint32_t framesInFlight, Paths paths, DeviceInfo deviceInfo)
    : m_width(width)
    , m_height(height)
    , m_renderedIterations(renderedIterations)
    , m_framesInFlight(framesInFlight)
    , m_paths(paths)
    , m_deviceInfo(deviceInfo)
{
    std::cout << "WithAovsInteropApp()" << std::endl;
}

WithAovsInteropApp::~WithAovsInteropApp()
{
    std::cout << "~WithAovsInteropApp()" << std::endl;
    for (auto f : m_fences) {
        RPRPP_CHECK(rprppVkDestroyFence(m_ppContext->getVkDevice(), f));
    }

    for (auto s : m_frameBuffersReleaseSemaphores) {
        RPRPP_CHECK(rprppVkDestroySemaphore(m_ppContext->getVkDevice(), s));
    }

    glfwDestroyWindow(m_window);
    glfwTerminate();
}

void WithAovsInteropApp::run()
{
    initWindow();
    findAdapter();
    intiSwapChain();
    initHybridProAndPostProcessing();
    resize(m_width, m_height);
    mainLoop();
}

void WithAovsInteropApp::initWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    m_window = glfwCreateWindow(m_width, m_height, "VkDx11 Interop", nullptr, nullptr);
    glfwGetWindowSize(m_window, &m_width, &m_height);
    m_hWnd = glfwGetWin32Window(m_window);
    glfwSetFramebufferSizeCallback(m_window, WithAovsInteropApp::onResize);
    glfwSetWindowUserPointer(m_window, this);
}

void WithAovsInteropApp::findAdapter()
{
    ComPtr<IDXGIFactory4> factory;
    DX_CHECK(CreateDXGIFactory2(0, IID_PPV_ARGS(&factory)));
    ComPtr<IDXGIFactory6> factory6;
    DX_CHECK(factory->QueryInterface(IID_PPV_ARGS(&factory6)));

    LUID* luid = reinterpret_cast<LUID*>(&m_deviceInfo.LUID[0]);
    SUCCEEDED(factory6->EnumAdapterByLuid(*luid, IID_PPV_ARGS(&m_adapter)));
}

void WithAovsInteropApp::intiSwapChain()
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

void WithAovsInteropApp::initHybridProAndPostProcessing()
{
    m_ppContext = std::make_unique<rprpp::wrappers::Context>(m_deviceInfo.index);
    m_postProcessing = std::make_unique<rprpp::wrappers::PostProcessing>(*m_ppContext);

    for (uint32_t i = 0; i < m_framesInFlight; i++) {
        RprPpVkSemaphore semaphore;
        RPRPP_CHECK(rprppVkCreateSemaphore(m_ppContext->getVkDevice(), &semaphore));
        m_frameBuffersReleaseSemaphores.push_back(semaphore);

        RprPpVkFence fence;
        RPRPP_CHECK(rprppVkCreateFence(m_ppContext->getVkDevice(), RPRPP_TRUE, &fence));
        m_fences.push_back(fence);
    }

    // set frame buffers realese to signal state
    RPRPP_CHECK(rprppVkQueueSubmitWaitAndSignal(m_ppContext->getVkQueue(), nullptr, m_frameBuffersReleaseSemaphores[1 % m_framesInFlight], nullptr));

    HybridProInteropInfo aovsInteropInfo = HybridProInteropInfo {
        .physicalDevice = m_ppContext->getVkPhysicalDevice(),
        .device = m_ppContext->getVkDevice(),
        .framesInFlight = m_framesInFlight,
        .frameBuffersReleaseSemaphores = m_frameBuffersReleaseSemaphores.data(),
    };

    m_hybridproRenderer = std::make_unique<HybridProRenderer>(m_deviceInfo.index, aovsInteropInfo, m_paths.hybridproDll, m_paths.hybridproCacheDir, m_paths.assetsDir);
    m_frameBuffersReadySemaphores = m_hybridproRenderer->getFrameBuffersReadySemaphores();
}

void WithAovsInteropApp::resize(int width, int height)
{
    if (m_width != width || m_height != height || m_backBuffer.Get() == nullptr) {
        m_deviceContex->OMSetRenderTargets(0, nullptr, nullptr);
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
        HANDLE sharedTextureHandle;
        DX_CHECK(m_sharedTextureResource->CreateSharedHandle(nullptr, GENERIC_ALL, nullptr, &sharedTextureHandle));

        D3D11_VIEWPORT viewport;
        ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

        viewport.Width = width;
        viewport.Height = height;
        m_deviceContex->RSSetViewports(1, &viewport);

        m_hybridproRenderer->resize(width, height);
        // post processing
        {
            RprPpImageDescription outputDesc = {
                .width = (uint32_t)width,
                .height = (uint32_t)height,
                .format = to_rprppformat(FORMAT),
            };
            m_output = std::make_unique<rprpp::wrappers::Image>(rprpp::wrappers::Image::create(*m_ppContext, outputDesc));
            m_dx11output = std::make_unique<rprpp::wrappers::Image>(rprpp::wrappers::Image::createImageFromDx11Texture(*m_ppContext, static_cast<RprPpDx11Handle>(sharedTextureHandle), outputDesc));
            RprPpImageDescription aovsDesc = {
                .width = (uint32_t)width,
                .height = (uint32_t)height,
                .format = RPRPP_IMAGE_FROMAT_R32G32B32A32_SFLOAT,
            };
            m_aovColor = std::make_unique<rprpp::wrappers::Image>(rprpp::wrappers::Image::createFromVkSampledImage(*m_ppContext, (RprPpVkImage)m_hybridproRenderer->getAovVkImage(RPR_AOV_COLOR), aovsDesc));
            m_aovOpacity = std::make_unique<rprpp::wrappers::Image>(rprpp::wrappers::Image::createFromVkSampledImage(*m_ppContext, (RprPpVkImage)m_hybridproRenderer->getAovVkImage(RPR_AOV_OPACITY), aovsDesc));
            m_aovShadowCatcher = std::make_unique<rprpp::wrappers::Image>(rprpp::wrappers::Image::createFromVkSampledImage(*m_ppContext, (RprPpVkImage)m_hybridproRenderer->getAovVkImage(RPR_AOV_SHADOW_CATCHER), aovsDesc));
            m_aovReflectionCatcher = std::make_unique<rprpp::wrappers::Image>(rprpp::wrappers::Image::createFromVkSampledImage(*m_ppContext, (RprPpVkImage)m_hybridproRenderer->getAovVkImage(RPR_AOV_REFLECTION_CATCHER), aovsDesc));
            m_aovMattePass = std::make_unique<rprpp::wrappers::Image>(rprpp::wrappers::Image::createFromVkSampledImage(*m_ppContext, (RprPpVkImage)m_hybridproRenderer->getAovVkImage(RPR_AOV_MATTE_PASS), aovsDesc));
            m_aovBackground = std::make_unique<rprpp::wrappers::Image>(rprpp::wrappers::Image::createFromVkSampledImage(*m_ppContext, (RprPpVkImage)m_hybridproRenderer->getAovVkImage(RPR_AOV_BACKGROUND), aovsDesc));
            m_postProcessing->setOutput(*m_output);
            m_postProcessing->setAovColor(*m_aovColor);
            m_postProcessing->setAovOpacity(*m_aovOpacity);
            m_postProcessing->setAovShadowCatcher(*m_aovShadowCatcher);
            m_postProcessing->setAovReflectionCatcher(*m_aovReflectionCatcher);
            m_postProcessing->setAovMattePass(*m_aovMattePass);
            m_postProcessing->setAovBackground(*m_aovBackground);
            m_postProcessing->setToneMapFocalLength(m_hybridproRenderer->getFocalLength() / 1000.0f);
            // m_postProcessing->setBloomEnabled(true);
            // m_postProcessing->setBloomRadius(0.1f);
            // m_postProcessing->setBloomThreshold(0.0f);
            // m_postProcessing->setBloomBrightnessScale(1.0f);
        }

        m_width = width;
        m_height = height;
    }
}

void WithAovsInteropApp::onResize(GLFWwindow* window, int width, int height)
{
    auto app = static_cast<WithAovsInteropApp*>(glfwGetWindowUserPointer(window));
    app->resize(width, height);
}

void WithAovsInteropApp::mainLoop()
{
    clock_t deltaTime = 0;
    unsigned int frames = 0;
    while (!glfwWindowShouldClose(m_window)) {
        clock_t beginFrame = clock();
        {
            glfwPollEvents();
            {
                for (unsigned int i = 0; i < m_renderedIterations; i++) {
                    // +1 because we get semaphore before rendering
                    uint32_t semaphoreIndex = (m_hybridproRenderer->getSemaphoreIndex() + 1) % m_framesInFlight;
                    RprPpVkFence fence = m_fences[semaphoreIndex];
                    RPRPP_CHECK(rprppVkWaitForFences(m_ppContext->getVkDevice(), 1, &fence, true, UINT64_MAX));
                    RPRPP_CHECK(rprppVkResetFences(m_ppContext->getVkDevice(), 1, &fence));

                    m_hybridproRenderer->render();
                    m_hybridproRenderer->flushFrameBuffers();

                    RprPpVkSemaphore aovsReadySemaphore = m_frameBuffersReadySemaphores[semaphoreIndex];
                    RprPpVkSemaphore aovsReleasedSemaphore = m_frameBuffersReleaseSemaphores[(semaphoreIndex + 1) % m_framesInFlight];

                    if (i < m_renderedIterations - 1) {
                        RPRPP_CHECK(rprppVkQueueSubmitWaitAndSignal(m_ppContext->getVkQueue(), aovsReadySemaphore, aovsReleasedSemaphore, fence));
                    } else {
                        m_postProcessing->run(aovsReadySemaphore, aovsReleasedSemaphore);
                        RPRPP_CHECK(rprppVkQueueSubmitWaitAndSignal(m_ppContext->getVkQueue(), nullptr, nullptr, fence));
                    }
                }

                m_ppContext->waitQueueIdle();
                m_ppContext->copyImage(m_output->get(), m_dx11output->get());
            }

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