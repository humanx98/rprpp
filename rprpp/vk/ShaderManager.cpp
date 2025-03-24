#include "ShaderManager.h"
#include "rprpp/Error.h"
#include <mutex>
#include <rprpp_config.h>
#include <shaderc/shaderc.hpp>
#include <boost/log/trivial.hpp>

namespace vk::helper {

vk::raii::ShaderModule ShaderManager::get(const vk::raii::Device& device,
    const std::string& shaderName,
    const char* source_text,
    size_t source_text_size,
    const std::unordered_map<std::string, std::string>& macroDefinitions)
{
    static std::mutex mutex;
    static std::unordered_map<std::string, std::vector<uint32_t>> compiledShaders;

    std::string key = shaderName;
    shaderc::CompileOptions options;
    options.SetOptimizationLevel(shaderc_optimization_level_performance);
    for (auto& it : macroDefinitions) {
        key += it.first + "_" + it.second + "_";
        options.AddMacroDefinition(it.first, it.second);
    }

    std::lock_guard<std::mutex> lock(mutex);
    auto it = compiledShaders.find(key);
    if (it == compiledShaders.end()) {
        shaderc::Compiler compiler;
        shaderc::SpvCompilationResult spv = compiler.CompileGlslToSpv(
            source_text,
            source_text_size,
            shaderc_glsl_compute_shader,
            shaderName.c_str(),
            options);

        if (spv.GetCompilationStatus() != shaderc_compilation_status_success) {
            BOOST_LOG_TRIVIAL(error) << spv.GetErrorMessage() << std::endl;
            throw rprpp::ShaderCompilationError("Shader compilation failed with: " + spv.GetErrorMessage());
        }

        compiledShaders[key] = std::move(std::vector(spv.cbegin(), spv.cend()));
        it = compiledShaders.find(key);
    }

    vk::ShaderModuleCreateInfo shaderModuleInfo({}, it->second);
    return vk::raii::ShaderModule(device, shaderModuleInfo);
}

vk::raii::ShaderModule ShaderManager::getBloomConvolve1dShader(const vk::raii::Device& device, const std::unordered_map<std::string, std::string>& macroDefinitions)
{
    return get(device,
        "bloom_convolve1d",
        RPRPP_bloom_convolve1d_SHADER,
        // size - null terminator
        sizeof(RPRPP_bloom_convolve1d_SHADER) - 1,
        macroDefinitions);
}

vk::raii::ShaderModule ShaderManager::getBloomConvolve2dShader(const vk::raii::Device& device, const std::unordered_map<std::string, std::string>& macroDefinitions)
{
    return get(device,
        "bloom_convolve2d",
        RPRPP_bloom_convolve2d_SHADER,
        // size - null terminator
        sizeof(RPRPP_bloom_convolve2d_SHADER) - 1,
        macroDefinitions);
}

vk::raii::ShaderModule ShaderManager::getBloomThresholdShader(const vk::raii::Device& device, const std::unordered_map<std::string, std::string>& macroDefinitions)
{
    return get(device,
        "blooom_threshold",
        RPRPP_bloom_threshold_SHADER,
        // size - null terminator
        sizeof(RPRPP_bloom_threshold_SHADER) - 1,
        macroDefinitions);
}

vk::raii::ShaderModule ShaderManager::getComposeColorShadowReflectionShader(const vk::raii::Device& device, const std::unordered_map<std::string, std::string>& macroDefinitions)
{
    return get(device,
        "compose_color_shadow_reflection",
        RPRPP_compose_color_shadow_reflection_SHADER,
        // size - null terminator
        sizeof(RPRPP_compose_color_shadow_reflection_SHADER) - 1,
        macroDefinitions);
}

vk::raii::ShaderModule ShaderManager::getComposeOpacityShadowShader(const vk::raii::Device& device, const std::unordered_map<std::string, std::string>& macroDefinitions)
{
    return get(device,
        "compose_opacity_shadow",
        RPRPP_compose_opacity_shadow_SHADER,
        // size - null terminator
        sizeof(RPRPP_compose_opacity_shadow_SHADER) - 1,
        macroDefinitions);
}

vk::raii::ShaderModule ShaderManager::getToneMapShader(const vk::raii::Device& device, const std::unordered_map<std::string, std::string>& macroDefinitions)
{
    return get(device,
        "tonemap",
        RPRPP_tonemap_SHADER,
        // size - null terminator
        sizeof(RPRPP_tonemap_SHADER) - 1,
        macroDefinitions);
}

}