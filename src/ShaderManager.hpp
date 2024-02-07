#pragma once

#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <shaderc/shaderc.hpp>
#include <vulkan/vulkan_raii.hpp>

namespace rprpp {

class ShaderManager {
public:
    vk::raii::ShaderModule get(const vk::raii::Device& device,
        const std::map<std::string, std::string>& macroDefinitions,
        const std::filesystem::path& path)
    {
        std::ifstream fglsl(path);
        std::string computeShaderGlsl((std::istreambuf_iterator<char>(fglsl)), std::istreambuf_iterator<char>());

        shaderc::Compiler compiler;
        shaderc::CompileOptions options;
        for (auto& it : macroDefinitions) {
            options.AddMacroDefinition(it.first, it.second);
        }
        options.SetOptimizationLevel(shaderc_optimization_level_performance);
        shaderc::SpvCompilationResult spv = compiler.CompileGlslToSpv(
            computeShaderGlsl,
            shaderc_glsl_compute_shader,
            "shader",
            options);

        if (spv.GetCompilationStatus() != shaderc_compilation_status_success) {
            std::cerr << spv.GetErrorMessage();
        }

        vk::ShaderModuleCreateInfo shaderModuleInfo({}, std::distance(spv.cbegin(), spv.cend()) * sizeof(uint32_t), spv.cbegin());
        return vk::raii::ShaderModule(device, shaderModuleInfo);
    }
};

}