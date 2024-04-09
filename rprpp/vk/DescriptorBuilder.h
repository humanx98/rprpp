#pragma once

#include "vk.h"
#include <vector>

namespace vk::helper {

class DescriptorBuilder {
public:
    void bindStorageImage(const vk::DescriptorImageInfo* imageInfo);
    void bindCombinedImageSampler(vk::DescriptorImageInfo* imageInfo);
    void bindUniformBuffer(vk::DescriptorBufferInfo* bufferInfo);

    [[nodiscard]]
    const std::vector<vk::DescriptorPoolSize>& poolSizes() const { return m_poolSizesCached; }

    [[nodiscard]]
    const std::vector<vk::DescriptorSetLayoutBinding>& bindings() const noexcept { return m_bindings; }

    [[nodiscard]]
    const std::vector<vk::WriteDescriptorSet>& writes() const noexcept { return m_writes; }

    void updateDescriptorSet(const vk::DescriptorSet& descriptorSet);

private:
    [[nodiscard]]
    vk::DescriptorPoolSize& findOrCreate(vk::DescriptorType type);

    std::vector<vk::DescriptorSetLayoutBinding> m_bindings;
    std::vector<vk::WriteDescriptorSet> m_writes;
    std::vector<vk::DescriptorPoolSize> m_poolSizesCached;
};

}