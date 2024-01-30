#pragma once

#include "PostProcessing.hpp"
#include "common.hpp"
#include <RadeonProRender.h>
#include <filesystem>
#include <map>
#include <memory>
#include <vector>
#include <windows.h>

struct FrameSemaphores {
    VkFence fence;
    VkSemaphore ready;
    VkSemaphore release;
};

class HybridProRenderer {
private:
    std::unique_ptr<PostProcessing> m_postProcessing = nullptr;
    std::vector<FrameSemaphores> m_semaphores;
    rpr_context m_context = nullptr;
    rpr_material_system m_matsys = nullptr;
    rpr_material_node m_uberv2 = nullptr;
    rpr_scene m_scene = nullptr;
    rpr_camera m_camera = nullptr;
    rpr_shape m_teapot = nullptr;
    rpr_image m_iblimage = nullptr;
    rpr_light m_light = nullptr;
    std::map<rpr_aov, rpr_framebuffer> m_aovs;
    std::map<rpr_aov, std::vector<uint8_t>> m_tmpAovs;

    const std::vector<uint8_t>& readAovBuff(rpr_aov aov);

public:
    HybridProRenderer(const Paths& paths,
        HANDLE sharedTextureHandle,
        rpr_uint w,
        rpr_uint h,
        GpuIndices gpuIndices);
    HybridProRenderer(const HybridProRenderer&&) = delete;
    HybridProRenderer(const HybridProRenderer&) = delete;
    HybridProRenderer& operator=(const HybridProRenderer&) = delete;
    ~HybridProRenderer();

    void render(rpr_uint iterations);
    void saveResultTo(const char* path, rpr_aov aov);
};