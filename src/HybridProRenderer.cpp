#include "HybridProRenderer.hpp"
#include "rpr_helper.hpp"
#include <RadeonProRender_VK.h>
#include <set>

inline constexpr int FramesInFlight = 3;

HybridProRenderer::HybridProRenderer(const Paths& paths,
    HANDLE sharedTextureHandle,
    rpr_uint width,
    rpr_uint height,
    GpuIndices gpuIndices)
{
    std::cout << "[HybridProRenderer] HybridProRenderer()" << std::endl;

    std::vector<rpr_context_properties> properties;
    rpr_creation_flags creation_flags = RPR_CREATION_FLAGS_ENABLE_GPU0;
    m_postProcessing = std::make_unique<PostProcessing>(paths, sharedTextureHandle, true, width, height, gpuIndices);

    // std::vector<VkSemaphore> releaseSemaphores;
    // releaseSemaphores.reserve(FramesInFlight);
    // m_semaphores.resize(FramesInFlight);
    // for (size_t i = 0; i < FramesInFlight; i++) {
    //     VkSemaphoreCreateInfo semaphoreInfo {};
    //     semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    //     VkFenceCreateInfo fenceInfo {};
    //     fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    //     fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    //     VK_CHECK(vkCreateSemaphore(m_postProcessing->getVkDevice(), &semaphoreInfo, nullptr, &m_semaphores[i].ready));
    //     VK_CHECK(vkCreateSemaphore(m_postProcessing->getVkDevice(), &semaphoreInfo, nullptr, &m_semaphores[i].release));
    //     VK_CHECK(vkCreateFence(m_postProcessing->getVkDevice(), &fenceInfo, nullptr, &m_semaphores[i].fence));
    //     releaseSemaphores.push_back(m_semaphores[i].release);
    // }

    // VkInteropInfo::VkInstance instance = {
    //     m_postProcessing->getVkPhysicalDevice(),
    //     m_postProcessing->getVkDevice()
    // };

    // VkInteropInfo interopInfo = {
    //     .instance_count = 1,
    //     .main_instance_index = 0,
    //     .frames_in_flight = FramesInFlight,
    //     .framebuffers_release_semaphores = releaseSemaphores.data(),
    //     .instances = &instance,
    // };

    // creation_flags |= RPR_CREATION_FLAGS_ENABLE_VK_INTEROP;
    // properties.push_back((rpr_context_properties)RPR_CONTEXT_CREATEPROP_VK_INTEROP_INFO);
    // properties.push_back((rpr_context_properties)&interopInfo);

    properties.push_back(0);

    rpr_int pluginId = rprRegisterPlugin(paths.hybridproDll.string().c_str());
    rpr_int plugins[] = { pluginId };
    RPR_CHECK(rprCreateContext(
        RPR_API_VERSION,
        plugins,
        sizeof(plugins) / sizeof(plugins[0]),
        creation_flags,
        properties.data(),
        paths.hybridproCacheDir.string().c_str(),
        &m_context));
    RPR_CHECK(rprContextSetActivePlugin(m_context, plugins[0]));
    RPR_CHECK(rprContextSetParameterByKey1u(m_context, RPR_CONTEXT_MAX_RECURSION, 10));

    RPR_CHECK(rprContextCreateScene(m_context, &m_scene));
    RPR_CHECK(rprContextSetScene(m_context, m_scene));

    // material
    {
        RPR_CHECK(rprContextCreateMaterialSystem(m_context, 0, &m_matsys));
        RPR_CHECK(rprMaterialSystemCreateNode(m_matsys, RPR_MATERIAL_NODE_UBERV2, &m_uberv2));
        RPR_CHECK(rprMaterialNodeSetInputFByKey(m_uberv2, RPR_MATERIAL_INPUT_UBER_DIFFUSE_COLOR, 0.7f, 0.2f, 0.0f, 1.0f));
        RPR_CHECK(rprMaterialNodeSetInputFByKey(m_uberv2, RPR_MATERIAL_INPUT_UBER_DIFFUSE_WEIGHT, 1.0f, 1.0f, 1.0f, 1.0f));
    }

    // Camera
    {
        RPR_CHECK(rprContextCreateCamera(m_context, &m_camera));
        RPR_CHECK(rprSceneSetCamera(m_scene, m_camera));
        RPR_CHECK(rprObjectSetName(m_camera, (rpr_char*)"camera"));
        RPR_CHECK(rprCameraSetMode(m_camera, RPR_CAMERA_MODE_PERSPECTIVE));
        RPR_CHECK(rprCameraLookAt(m_camera,
            8.0f, -8.0f, 8.0f, // pos
            0.0f, 0.0f, 0.0f, // at
            0.0f, -1.0f, 0.0f // up
            ));
        const float sensorHeight = 24.0f;
        float aspectRatio = (float)width / height;
        RPR_CHECK(rprCameraSetSensorSize(m_camera, sensorHeight * aspectRatio, sensorHeight));
    }

    // shape
    {
        auto teapotShapePath = paths.assetsDir / "teapot.obj";
        m_teapot = ImportOBJ(teapotShapePath.string(), m_scene, m_context);
        RPR_CHECK(rprShapeSetMaterial(m_teapot, m_uberv2));
    }

    // ibl image
    {
        auto iblPath = paths.assetsDir / "envLightImage.exr";
        RPR_CHECK(rprContextCreateImageFromFile(m_context, iblPath.string().c_str(), &m_iblimage));
        RPR_CHECK(rprObjectSetName(m_iblimage, "iblimage"));
    }

    // env light
    {
        RPR_CHECK(rprContextCreateEnvironmentLight(m_context, &m_light));
        RPR_CHECK(rprObjectSetName(m_light, "light"));
        RPR_CHECK(rprEnvironmentLightSetImage(m_light, m_iblimage));
        rpr_float float_P16_4[] = {
            -1.0f, -0.0f, -0.0f, 0.0f,
            0.0f, -0.0f, 1.0f, 0.0f,
            -0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };
        RPR_CHECK(rprLightSetTransform(m_light, false, (rpr_float*)&float_P16_4));
        RPR_CHECK(rprSceneSetEnvironmentLight(m_scene, m_light));
    }

    // aovs
    {
        std::set<rpr_aov> aovs = {
            RPR_AOV_COLOR,
            RPR_AOV_OPACITY,
            RPR_AOV_SHADOW_CATCHER,
            RPR_AOV_REFLECTION_CATCHER,
            RPR_AOV_MATTE_PASS,
            RPR_AOV_BACKGROUND,
        };
        const rpr_framebuffer_desc desc = { width, height };
        const rpr_framebuffer_format fmt = { 4, RPR_COMPONENT_TYPE_FLOAT32 };
        for (auto aov : aovs) {
            rpr_framebuffer fb;
            RPR_CHECK(rprContextCreateFrameBuffer(m_context, fmt, &desc, &fb));
            RPR_CHECK(rprContextSetAOV(m_context, aov, fb));
            m_aovs[aov] = fb;
        }
    }
}

HybridProRenderer::~HybridProRenderer()
{
    std::cout << "[HybridProRenderer] ~HybridProRenderer()" << std::endl;
    for (auto& s : m_semaphores) {
        vkDestroySemaphore(m_postProcessing->getVkDevice(), s.ready, nullptr);
        vkDestroySemaphore(m_postProcessing->getVkDevice(), s.release, nullptr);
        vkDestroyFence(m_postProcessing->getVkDevice(), s.fence, nullptr);
    }

    RPR_CHECK(rprSceneDetachLight(m_scene, m_light));
    RPR_CHECK(rprSceneDetachShape(m_scene, m_teapot));
    for (auto& aov : m_aovs) {
        RPR_CHECK(rprObjectDelete(aov.second));
    }
    RPR_CHECK(rprObjectDelete(m_light));
    m_light = nullptr;
    RPR_CHECK(rprObjectDelete(m_iblimage));
    m_iblimage = nullptr;
    RPR_CHECK(rprObjectDelete(m_teapot));
    m_teapot = nullptr;
    RPR_CHECK(rprObjectDelete(m_camera));
    m_camera = nullptr;
    RPR_CHECK(rprObjectDelete(m_scene));
    m_scene = nullptr;
    RPR_CHECK(rprObjectDelete(m_uberv2));
    m_uberv2 = nullptr;
    RPR_CHECK(rprObjectDelete(m_matsys));
    m_matsys = nullptr;
    CheckNoLeak(m_context);
    RPR_CHECK(rprObjectDelete(m_context));
    m_context = nullptr;
}

const std::vector<uint8_t>& HybridProRenderer::readAovBuff(rpr_aov aovKey)
{
    rpr_framebuffer aov = m_aovs[aovKey];
    size_t size;
    RPR_CHECK(rprFrameBufferGetInfo(aov, RPR_FRAMEBUFFER_DATA, 0, nullptr, &size));

    if (!m_tmpAovs.contains(aovKey)) {
        m_tmpAovs[aovKey] = {};
    }

    auto& tmpAov = m_tmpAovs[aovKey];
    if (tmpAov.size() != size) {
        tmpAov.resize(size);
    }

    uint8_t* buff = nullptr;
    RPR_CHECK(rprFrameBufferGetInfo(aov, RPR_FRAMEBUFFER_DATA, size, tmpAov.data(), nullptr));
    return tmpAov;
}

void HybridProRenderer::render(rpr_uint iterations)
{
    RPR_CHECK(rprContextSetParameterByKey1u(m_context, RPR_CONTEXT_ITERATIONS, iterations));
    RPR_CHECK(rprContextRender(m_context));

    m_postProcessing->updateAovColor(readAovBuff(RPR_AOV_COLOR));
    m_postProcessing->updateAovOpacity(readAovBuff(RPR_AOV_OPACITY));
    m_postProcessing->updateAovShadowCatcher(readAovBuff(RPR_AOV_SHADOW_CATCHER));
    m_postProcessing->updateAovReflectionCatcher(readAovBuff(RPR_AOV_REFLECTION_CATCHER));
    m_postProcessing->updateAovMattePass(readAovBuff(RPR_AOV_MATTE_PASS));
    m_postProcessing->updateAovBackground(readAovBuff(RPR_AOV_BACKGROUND));
    m_postProcessing->apply();
}

void HybridProRenderer::saveResultTo(const char* path, rpr_aov aov)
{
    RPR_CHECK(rprFrameBufferSaveToFile(m_aovs[aov], path));
}