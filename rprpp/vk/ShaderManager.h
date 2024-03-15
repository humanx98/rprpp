#pragma once

#include "vk.h"
#include <unordered_map>

namespace vk::helper {

class ShaderManager {
private:
    vk::raii::ShaderModule get(const vk::raii::Device& device,
        const std::string& shaderName,
        const char* source_text,
        size_t source_text_size,
        const std::unordered_map<std::string, std::string>& macroDefinitions);

public:
    vk::raii::ShaderModule getBloomHorizontalShader(const vk::raii::Device& device, const std::unordered_map<std::string, std::string>& macroDefinitions);
    vk::raii::ShaderModule getBloomVerticalShader(const vk::raii::Device& device, const std::unordered_map<std::string, std::string>& macroDefinitions);
    vk::raii::ShaderModule getComposeColorShadowReflectionShader(const vk::raii::Device& device, const std::unordered_map<std::string, std::string>& macroDefinitions);
    vk::raii::ShaderModule getComposeOpacityShadowShader(const vk::raii::Device& device, const std::unordered_map<std::string, std::string>& macroDefinitions);
    vk::raii::ShaderModule getToneMapShader(const vk::raii::Device& device, const std::unordered_map<std::string, std::string>& macroDefinitions);
};

}