#pragma once

#include "vk.h"
#include <map>

namespace rprpp {

class ShaderManager {
public:
    vk::raii::ShaderModule get(const vk::raii::Device& device, const std::map<std::string, std::string>& macroDefinitions);
};

}