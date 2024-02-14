#include "DescriptorBuilder.h"

namespace rprpp {

    std::vector<vk::DescriptorPoolSize> DescriptorBuilder::poolSizes()
    {
        std::vector<vk::DescriptorPoolSize> poolSizes;
        poolSizes.reserve(m_poolSizes.size());
        for (auto& ps : m_poolSizes) {
            poolSizes.push_back(vk::DescriptorPoolSize(ps.first, ps.second));
        }
        return poolSizes;
    }

    void DescriptorBuilder::bindStorageImage(const vk::DescriptorImageInfo* imageInfo)
    {
        uint32_t bindingIndex = static_cast<uint32_t>(bindings.size());
        vk::DescriptorType type = vk::DescriptorType::eStorageImage;
        vk::DescriptorSetLayoutBinding binding;
        binding.binding = bindingIndex;
        binding.descriptorType = type;
        binding.descriptorCount = 1;
        binding.stageFlags = vk::ShaderStageFlagBits::eCompute;
        bindings.push_back(binding);

        vk::WriteDescriptorSet write;
        write.dstBinding = bindingIndex;
        write.descriptorCount = 1;
        write.descriptorType = type;
        write.pImageInfo = imageInfo;
        writes.push_back(write);

        ++m_poolSizes[type];
    }

    void DescriptorBuilder::bindUniformBuffer(vk::DescriptorBufferInfo* bufferInfo)
    {
        uint32_t bindingIndex = static_cast<uint32_t>(bindings.size());
        vk::DescriptorType type = vk::DescriptorType::eUniformBuffer;
        vk::DescriptorSetLayoutBinding binding;
        binding.binding = bindingIndex;
        binding.descriptorCount = 1;
        binding.descriptorType = type;
        binding.pImmutableSamplers = nullptr;
        binding.stageFlags = vk::ShaderStageFlagBits::eCompute;
        bindings.push_back(binding);

        vk::WriteDescriptorSet write;
        write.dstBinding = bindingIndex;
        write.descriptorCount = 1;
        write.descriptorType = type;
        write.pBufferInfo = bufferInfo;
        writes.push_back(write);

        ++m_poolSizes[type];
    }
}
