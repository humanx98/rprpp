#pragma once

#include "vk.h"
#include <map>
#include <optional>
#include <vector>

namespace rprpp {

class DescriptorBuilder {
private:
    std::map<vk::DescriptorType, size_t> m_poolSizes;

public:
    std::vector<vk::DescriptorSetLayoutBinding> bindings;
    std::vector<vk::WriteDescriptorSet> writes;

    std::vector<vk::DescriptorPoolSize> poolSizes();
    void bindStorageImage(const vk::DescriptorImageInfo* imageInfo);
    void bindUniformBuffer(vk::DescriptorBufferInfo* bufferInfo);
};

}