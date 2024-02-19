#pragma once

#include "vk.h"
#include <unordered_map>

namespace rprpp {

class ShaderManager {
public:
    vk::raii::ShaderModule get(const vk::raii::Device& device, const std::unordered_map<std::string, std::string>& macroDefinitions);
};

}