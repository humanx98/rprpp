#pragma once

#include "common.hpp"
#include <RadeonProRender.h>
#include <filesystem>
#include <map>
#include <memory>
#include <vector>
#include <windows.h>

class HybridProRenderer {
private:
    uint32_t m_width;
    uint32_t m_height;
    rpr_context m_context = nullptr;
    rpr_material_system m_matsys = nullptr;
    rpr_material_node m_teapotMaterial = nullptr;
    rpr_material_node m_shadowReflectionCatcherMaterial = nullptr;
    rpr_scene m_scene = nullptr;
    rpr_camera m_camera = nullptr;
    rpr_shape m_teapot = nullptr;
    rpr_shape m_shadowReflectionCatcherPlane = nullptr;
    rpr_image m_iblimage = nullptr;
    rpr_light m_light = nullptr;
    std::map<rpr_aov, rpr_framebuffer> m_aovs;
    std::vector<uint8_t> m_tmpAovBuff;

    const std::vector<uint8_t>& readAovBuff(rpr_aov aov);

public:
    HybridProRenderer(const Paths& paths,
        uint32_t width,
        uint32_t height,
        GpuIndices gpuIndices);
    HybridProRenderer(const HybridProRenderer&&) = delete;
    HybridProRenderer(const HybridProRenderer&) = delete;
    HybridProRenderer& operator=(const HybridProRenderer&) = delete;
    ~HybridProRenderer();

    void resize(uint32_t width, uint32_t height);
    void render(uint32_t iterations);
    void saveResultTo(const char* path, rpr_aov aov);
    float getFocalLength();

    inline rpr_framebuffer getAov(rpr_aov aov)
    {
        return m_aovs[aov];
    }
};