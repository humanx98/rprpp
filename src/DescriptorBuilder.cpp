#include "DescriptorBuilder.h"

namespace rprpp {

    void DescriptorBuilder::updateDescriptorSet(const vk::DescriptorSet& descriptorSet)
    {
        for (auto& write : m_writes) {
            write.dstSet = descriptorSet;
        }
    }

    void DescriptorBuilder::bindStorageImage(const vk::DescriptorImageInfo* imageInfo)
    {
        uint32_t bindingIndex = static_cast<uint32_t>(m_bindings.size());
        vk::DescriptorType type = vk::DescriptorType::eStorageImage;

        vk::DescriptorSetLayoutBinding binding;
        binding.binding = bindingIndex;
        binding.descriptorType = type;
        binding.descriptorCount = 1;
        binding.stageFlags = vk::ShaderStageFlagBits::eCompute;
        m_bindings.push_back(binding);

        vk::WriteDescriptorSet write;
        write.dstBinding = bindingIndex;
        write.descriptorCount = 1;
        write.descriptorType = type;
        write.pImageInfo = imageInfo;
        m_writes.push_back(write);

        vk::DescriptorPoolSize& poolSize = findOrCreate(type);
        poolSize.descriptorCount++;
    }

    void DescriptorBuilder::bindCombinedImageSampler(vk::DescriptorImageInfo* imageInfo)
    {
        uint32_t bindingIndex = static_cast<uint32_t>(m_bindings.size());
        vk::DescriptorType type = vk::DescriptorType::eCombinedImageSampler;

        vk::DescriptorSetLayoutBinding binding;
        binding.binding = bindingIndex;
        binding.descriptorType = type;
        binding.descriptorCount = 1;
        binding.stageFlags = vk::ShaderStageFlagBits::eCompute;
        m_bindings.push_back(binding);

        vk::WriteDescriptorSet write;
        write.dstBinding = bindingIndex;
        write.descriptorCount = 1;
        write.descriptorType = type;
        write.pImageInfo = imageInfo;
        m_writes.push_back(write);

        vk::DescriptorPoolSize& poolSize = findOrCreate(type);
        poolSize.descriptorCount++;
    }

    void DescriptorBuilder::bindUniformBuffer(vk::DescriptorBufferInfo* bufferInfo)
    {
        uint32_t bindingIndex = static_cast<uint32_t>(m_bindings.size());
        vk::DescriptorType type = vk::DescriptorType::eUniformBuffer;

        vk::DescriptorSetLayoutBinding binding;
        binding.binding = bindingIndex;
        binding.descriptorCount = 1;
        binding.descriptorType = type;
        binding.pImmutableSamplers = nullptr;
        binding.stageFlags = vk::ShaderStageFlagBits::eCompute;
        m_bindings.push_back(binding);
        
        vk::WriteDescriptorSet write;
        write.dstBinding = bindingIndex;
        write.descriptorCount = 1;
        write.descriptorType = type;
        write.pBufferInfo = bufferInfo;
        m_writes.push_back(write);

        vk::DescriptorPoolSize& poolSize = findOrCreate(type);
        poolSize.descriptorCount++;
    }

    vk::DescriptorPoolSize& DescriptorBuilder::findOrCreate(vk::DescriptorType type)
    {
        auto same_type = [&type](const vk::DescriptorPoolSize& p) { return p.type == type; };
        auto iter = std::find_if(m_poolSizesCached.begin(), m_poolSizesCached.end(), same_type);

        if (iter == m_poolSizesCached.end()) {
            m_poolSizesCached.emplace_back(type, 0);

            iter = m_poolSizesCached.end() - 1;
        }

       return *iter;
    }
}
