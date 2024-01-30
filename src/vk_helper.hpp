#pragma once

#include <map>
#include <optional>
#include <vector>
#include <vulkan/vulkan_raii.hpp>

namespace vk::helper {

class DescriptorBuilder {
private:
    std::map<vk::DescriptorType, size_t> m_poolSizes;

public:
    std::vector<vk::DescriptorSetLayoutBinding> bindings;
    std::vector<vk::WriteDescriptorSet> writes;

    std::vector<vk::DescriptorPoolSize> poolSizes()
    {
        std::vector<vk::DescriptorPoolSize> poolSizes;
        poolSizes.reserve(m_poolSizes.size());
        for (auto& ps : m_poolSizes) {
            poolSizes.push_back(vk::DescriptorPoolSize(ps.first, ps.second));
        }
        return poolSizes;
    }

    void bindStorageImage(vk::DescriptorImageInfo* dii)
    {
        uint32_t binding = static_cast<uint32_t>(bindings.size());
        vk::DescriptorType type = vk::DescriptorType::eStorageImage;
        vk::DescriptorSetLayoutBinding bind;
        bind.binding = binding;
        bind.descriptorType = type;
        bind.descriptorCount = 1;
        bind.stageFlags = vk::ShaderStageFlagBits::eCompute;
        bindings.push_back(bind);

        vk::WriteDescriptorSet write;
        write.dstBinding = binding;
        write.descriptorCount = 1;
        write.descriptorType = type;
        write.pImageInfo = dii;
        writes.push_back(write);

        ++m_poolSizes[type];
    }
};

}