#include "ShaderManager.h"
#include "Error.h"
#include <mutex>
#include <rprpp_config.h>
#include <shaderc/shaderc.hpp>

namespace rprpp {

vk::raii::ShaderModule ShaderManager::get(const vk::raii::Device& device, const std::unordered_map<std::string, std::string>& macroDefinitions)
{
    static std::mutex mutex;
    static std::unordered_map<std::string, std::vector<uint32_t>> compiledShaders;

    std::string key;
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
            RPRPP_POSTPROCESSING_SHADER,
            // size - null terminator
            sizeof(RPRPP_POSTPROCESSING_SHADER) - 1,
            shaderc_glsl_compute_shader,
            "shader",
            options);

        if (spv.GetCompilationStatus() != shaderc_compilation_status_success) {
            throw ShaderCompilationError("Shader compilation failed with: " + spv.GetErrorMessage());
        }

        compiledShaders[key] = std::move(std::vector(spv.cbegin(), spv.cend()));
        it = compiledShaders.find(key);
    }

    vk::ShaderModuleCreateInfo shaderModuleInfo({}, it->second);
    return vk::raii::ShaderModule(device, shaderModuleInfo);
}

}