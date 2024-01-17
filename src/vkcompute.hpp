#pragma once

#include <iostream>
#include <assert.h>
#include <lodepng.h>
#include <shaderc/shaderc.hpp>
#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>
#include <winnt.h>

const int WORKGROUP_SIZE = 32;

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

#define VK_CHECK_RESULT(f)                                                                \
    {                                                                                     \
        VkResult res = (f);                                                               \
        if (res != VK_SUCCESS) {                                                          \
            printf("Fatal : VkResult is %d in %s at line %d\n", res, __FILE__, __LINE__); \
            assert(res == VK_SUCCESS);                                                    \
        }                                                                                 \
    }

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

class VkCompute {
private:
    int width, height;
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;
    VkShaderModule computeShaderModule;
    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;
    VkDescriptorPool descriptorPool;
    VkDescriptorSet descriptorSet;
    VkDescriptorSetLayout descriptorSetLayout;
    VkImage sharedOutput;
    VkImageView sharedOutputView;
    VkDeviceMemory sharedOutputMemory;
    VkQueue queue;
    uint32_t queueFamilyIndex;
    std::vector<const char*> enabledLayers;

public:
    void init(HANDLE sharedTextureHandle, int w, int h)
    {
        width = w;
        height = h;
        createInstance();
        setupDebugMessenger();
        findPhysicalDevice();
        createDevice();
        createDescriptorSetLayout();
        createComputePipeline();
        createCommandBuffer();
        createSharedOutput(sharedTextureHandle);
        createDescriptorSet();
    }

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
    }

    void setupDebugMessenger() {
        if (!enableValidationLayers) return;

        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        populateDebugMessengerCreateInfo(createInfo);

        if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
            throw std::runtime_error("failed to set up debug messenger!");
        }
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }

    void createInstance()
    {
        VkApplicationInfo applicationInfo = {};
        applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        applicationInfo.pApplicationName = "Hello world app";
        applicationInfo.applicationVersion = 0;
        applicationInfo.pEngineName = "awesomeengine";
        applicationInfo.engineVersion = 0;
        applicationInfo.apiVersion = VK_API_VERSION_1_1;

        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.flags = 0;
        createInfo.pApplicationInfo = &applicationInfo;


        std::vector<const char*> enabledExtensions;
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        if (enableValidationLayers) {
            uint32_t layerCount;
            VK_CHECK_RESULT(vkEnumerateInstanceLayerProperties(&layerCount, nullptr));
            std::vector<VkLayerProperties> layerProperties(layerCount);
            VK_CHECK_RESULT(vkEnumerateInstanceLayerProperties(&layerCount, layerProperties.data()));

            bool foundLayer = false;
            for (VkLayerProperties& prop : layerProperties) {
                if (strcmp("VK_LAYER_KHRONOS_validation", prop.layerName) == 0) {
                    foundLayer = true;
                    break;
                }
            }

            if (!foundLayer) {
                throw std::runtime_error("Layer VK_LAYER_KHRONOS_validation not supported\n");
            }
            enabledLayers.push_back("VK_LAYER_KHRONOS_validation");

            uint32_t extensionCount;
            VK_CHECK_RESULT(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr));
            std::vector<VkExtensionProperties> extensionProperties(extensionCount);
            VK_CHECK_RESULT(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensionProperties.data()));

            bool foundDebugUtilsExtension = false;
            bool foundExternalMemoryExtension = false;
            bool foundExternalSemaphoreExtension = false;
            for (VkExtensionProperties& prop : extensionProperties) {
                if (strcmp(VK_EXT_DEBUG_UTILS_EXTENSION_NAME, prop.extensionName) == 0) {
                    foundDebugUtilsExtension = true;
                } else if (strcmp(VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME, prop.extensionName) == 0) {
                    foundExternalMemoryExtension = true;
                } else if (strcmp(VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME, prop.extensionName) == 0) {
                    foundExternalSemaphoreExtension = true;
                }
            }

            if (!foundDebugUtilsExtension) {
                throw std::runtime_error("Extension VK_EXT_DEBUG_UTILS_EXTENSION_NAME not supported\n");
            }
            else if (!foundExternalMemoryExtension) {
                throw std::runtime_error("Extension VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME not supported\n");
            }
            else if (!foundExternalSemaphoreExtension) {
                throw std::runtime_error("Extension VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME not supported\n");
            }

            enabledExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            enabledExtensions.push_back(VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME);
            enabledExtensions.push_back(VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME);
            populateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
        }
        else {
            createInfo.pNext = nullptr;
        }

        createInfo.enabledLayerCount = enabledLayers.size();
        createInfo.ppEnabledLayerNames = enabledLayers.data();
        createInfo.enabledExtensionCount = enabledExtensions.size();
        createInfo.ppEnabledExtensionNames = enabledExtensions.data();
        VK_CHECK_RESULT(vkCreateInstance(&createInfo, nullptr, &instance));
    }

    void findPhysicalDevice()
    {
        uint32_t deviceCount;
        VK_CHECK_RESULT(vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr));
        if (deviceCount == 0) {
            throw std::runtime_error("could not find a device with vulkan support");
        }
        std::vector<VkPhysicalDevice> devices(deviceCount);
        VK_CHECK_RESULT(vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data()));


        std::cout << "[vkcompute.hpp] " << "All Physical devices:" << std::endl;
        int maxScore = -1;
        VkPhysicalDevice wantedDevice = nullptr;
        VkPhysicalDeviceProperties wantedDeviceProps;
        for (VkPhysicalDevice d : devices) {
            VkPhysicalDeviceProperties props;
            vkGetPhysicalDeviceProperties(d, &props);
            std::cout << "[vkcompute.hpp] " << "\t--" << props.deviceName;
            
            int score = 0;
            switch (props.deviceType) {
            case VK_PHYSICAL_DEVICE_TYPE_CPU:
                {
                    score += 1;
                    std::cout << ", deviceType = CPU";
                    break;
                }
            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                {
                    score += 20;
                    std::cout << ", deviceType = DGPU";
                    break;
                }
            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                {
                    score += 10;
                    std::cout << ", deviceType = IGPU";
                    break;
                }
            }

            std::cout << std::endl;

            if (score > maxScore) {
                wantedDevice = d;
                wantedDeviceProps = props;
                maxScore = score;
            }
        }

        physicalDevice = wantedDevice;
        std::cout << "[vkcompute.hpp] " 
            << "Selected Device (with score = " << maxScore << "): "
            << wantedDeviceProps.deviceName 
            << std::endl;
    }

    uint32_t getComputeQueueFamilyIndex()
    {
        uint32_t queueFamilyCount;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());
        uint32_t i = 0;
        for (; i < queueFamilies.size(); ++i) {
            VkQueueFamilyProperties props = queueFamilies[i];
            if (props.queueCount > 0 && (props.queueFlags & VK_QUEUE_COMPUTE_BIT)) {
                break;
            }
        }

        if (i == queueFamilies.size()) {
            throw std::runtime_error("could not find a queue family that supports operations");
        }

        return i;
    }

    void createDevice()
    {
        uint32_t extensionCount;
        VK_CHECK_RESULT(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr));
        std::vector<VkExtensionProperties> extensionProperties(extensionCount);
        VK_CHECK_RESULT(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, extensionProperties.data()));

        std::vector<const char*> enabledExtensions;
        bool foundExternalMemoryExtension = false;
        bool foundExternalSemaphoreExtension = false;
        bool foundExternalMemoryWin32Extension = false;
        bool foundExternalSemaphoreWin32Extension = false;
        for (VkExtensionProperties& prop : extensionProperties) {
            if (strcmp(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME, prop.extensionName) == 0) {
                foundExternalMemoryExtension = true;
            } else if (strcmp(VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME, prop.extensionName) == 0) {
                foundExternalSemaphoreExtension = true;
            } else if (strcmp(VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME, prop.extensionName) == 0) {
                foundExternalMemoryWin32Extension = true;
            } else if (strcmp(VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME, prop.extensionName) == 0) {
                foundExternalSemaphoreWin32Extension = true;
            }
        }

        if (!foundExternalMemoryExtension) {
            throw std::runtime_error("Extension VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME not supported\n");
        } else if (!foundExternalSemaphoreExtension) {
            throw std::runtime_error("Extension VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME not supported\n");
        } else if (!foundExternalMemoryWin32Extension) {
            throw std::runtime_error("Extension VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME not supported\n");
        } else if (!foundExternalSemaphoreWin32Extension) {
            throw std::runtime_error("Extension VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME not supported\n");
        }

        enabledExtensions.push_back(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME);
        enabledExtensions.push_back(VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME);
        enabledExtensions.push_back(VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME);
        enabledExtensions.push_back(VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME);

        queueFamilyIndex = getComputeQueueFamilyIndex();
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
        queueCreateInfo.queueCount = 1;
        float queuePriorities = 1.0;
        queueCreateInfo.pQueuePriorities = &queuePriorities;

        VkDeviceCreateInfo deviceCreateInfo = {};
        VkPhysicalDeviceFeatures deviceFeatures = {};

        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.enabledLayerCount = enabledLayers.size();
        deviceCreateInfo.ppEnabledLayerNames = enabledLayers.data();
        deviceCreateInfo.enabledExtensionCount = enabledExtensions.size();
        deviceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();
        deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
        deviceCreateInfo.queueCreateInfoCount = 1;
        deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

        VK_CHECK_RESULT(vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device));
        vkGetDeviceQueue(device, queueFamilyIndex, 0, &queue);
    }

    uint32_t findMemoryType(uint32_t memoryTypeBits, VkMemoryPropertyFlags properties)
    {
        VkPhysicalDeviceMemoryProperties memoryProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);
        for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i) {
            if ((memoryTypeBits & (1 << i)) && ((memoryProperties.memoryTypes[i].propertyFlags & properties) == properties))
                return i;
        }
        return -1;
    }

    VkCommandBuffer beginSingleTimeCommands()
    {
        VkCommandBufferAllocateInfo allocInfo {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = commandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer singleTimeCommandBuffer;
        VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &allocInfo, &singleTimeCommandBuffer));

        VkCommandBufferBeginInfo beginInfo {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        VK_CHECK_RESULT(vkBeginCommandBuffer(singleTimeCommandBuffer, &beginInfo));

        return singleTimeCommandBuffer;
    }

    void endSingleTimeCommands(VkCommandBuffer singleTimeCommandBuffer)
    {
        VK_CHECK_RESULT(vkEndCommandBuffer(singleTimeCommandBuffer));

        VkSubmitInfo submitInfo {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &singleTimeCommandBuffer;

        VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));
        VK_CHECK_RESULT(vkQueueWaitIdle(queue));

        vkFreeCommandBuffers(device, commandPool, 1, &singleTimeCommandBuffer);
    }

    void transitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout)
    {
        VkCommandBuffer singleTimeCommandBuffer = beginSingleTimeCommands();

        VkImageMemoryBarrier barrier {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.srcAccessMask = VK_ACCESS_NONE_KHR;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        vkCmdPipelineBarrier(
            singleTimeCommandBuffer,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier);

        endSingleTimeCommands(singleTimeCommandBuffer);
    }

    void createSharedOutput(HANDLE sharedTextureHandle)
    {
        auto format = VK_FORMAT_R8G8B8A8_UNORM;
        VkExternalMemoryImageCreateInfo emici = {};
        emici.sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO;
        emici.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_BIT;
        VkImageCreateInfo ici = {};
        ici.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        ici.pNext = &emici;
        ici.imageType = VK_IMAGE_TYPE_2D;
        ici.extent.width = width;
        ici.extent.height = height;
        ici.extent.depth = 1;
        ici.mipLevels = 1;
        ici.arrayLayers = 1;
        ici.format = format;
        ici.tiling = VK_IMAGE_TILING_OPTIMAL;
        ici.usage = VK_IMAGE_USAGE_STORAGE_BIT;
        ici.samples = VK_SAMPLE_COUNT_1_BIT;
        VK_CHECK_RESULT(vkCreateImage(device, &ici, nullptr, &sharedOutput));

        VkMemoryDedicatedRequirements mdr = {};
        mdr.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS;
        VkMemoryRequirements2 mr2 = {};
        mr2.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
        mr2.pNext = &mdr;
        VkImageMemoryRequirementsInfo2 imri2 = {};
        imri2.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2;
        imri2.image = sharedOutput;
        vkGetImageMemoryRequirements2(device, &imri2, &mr2);

        VkMemoryDedicatedAllocateInfo mdai = {};
        mdai.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO;
        mdai.image = sharedOutput;
        VkImportMemoryWin32HandleInfoKHR imw32hi = {};
        imw32hi.sType = VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_KHR;
        imw32hi.pNext = &mdai;
        imw32hi.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_BIT;
        imw32hi.handle = sharedTextureHandle;
        VkMemoryAllocateInfo mai = {};
        mai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        mai.pNext = &imw32hi;
        mai.allocationSize = mr2.memoryRequirements.size;
        mai.memoryTypeIndex = findMemoryType(mr2.memoryRequirements.memoryTypeBits, VK_MEMORY_HEAP_DEVICE_LOCAL_BIT);
        VK_CHECK_RESULT(vkAllocateMemory(device, &mai, nullptr, &sharedOutputMemory));
        VK_CHECK_RESULT(vkBindImageMemory(device, sharedOutput, sharedOutputMemory, 0));

        VkImageViewCreateInfo ivci = {};
        ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        ivci.image = sharedOutput;
        ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        ivci.format = format;
        ivci.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        ivci.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        ivci.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        ivci.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        ivci.subresourceRange.baseMipLevel = 0;
        ivci.subresourceRange.levelCount = 1;
        ivci.subresourceRange.baseArrayLayer = 0;
        ivci.subresourceRange.layerCount = 1;
        VK_CHECK_RESULT(vkCreateImageView(device, &ivci, nullptr, &sharedOutputView));

        transitionImageLayout(sharedOutput, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
    }

    void createDescriptorSetLayout()
    {
        VkDescriptorSetLayoutBinding descriptorSetLayoutBinding = {};
        descriptorSetLayoutBinding.binding = 0;
        descriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        descriptorSetLayoutBinding.descriptorCount = 1;
        descriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};
        descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayoutCreateInfo.bindingCount = 1;
        descriptorSetLayoutCreateInfo.pBindings = &descriptorSetLayoutBinding;
        VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCreateInfo, nullptr, &descriptorSetLayout));
    }

    void createDescriptorSet()
    {
        VkDescriptorPoolSize descriptorPoolSize = {};
        descriptorPoolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        descriptorPoolSize.descriptorCount = 1;

        VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
        descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptorPoolCreateInfo.maxSets = 1;
        descriptorPoolCreateInfo.poolSizeCount = 1;
        descriptorPoolCreateInfo.pPoolSizes = &descriptorPoolSize;
        VK_CHECK_RESULT(vkCreateDescriptorPool(device, &descriptorPoolCreateInfo, nullptr, &descriptorPool));

        VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
        descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        descriptorSetAllocateInfo.descriptorPool = descriptorPool;
        descriptorSetAllocateInfo.descriptorSetCount = 1;
        descriptorSetAllocateInfo.pSetLayouts = &descriptorSetLayout;
        VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, &descriptorSet));

        VkDescriptorImageInfo dii = {};
        dii.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        dii.imageView = sharedOutputView;
        dii.sampler = nullptr;

        VkWriteDescriptorSet writeDescriptorSet = {};
        writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet.dstSet = descriptorSet;
        writeDescriptorSet.dstBinding = 0;
        writeDescriptorSet.descriptorCount = 1;
        writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        writeDescriptorSet.pImageInfo = &dii;
        vkUpdateDescriptorSets(device, 1, &writeDescriptorSet, 0, nullptr);
    }

    void createComputePipeline()
    {
        std::string computeShaderGlsl = R"(#version 450
#define WIDTH 1600
#define HEIGHT 1000
#define WORKGROUP_SIZE 32
layout (local_size_x = WORKGROUP_SIZE, local_size_y = WORKGROUP_SIZE, local_size_z = 1 ) in;

layout (binding = 0, rgba8) uniform image2D outputBuffer;

void main() {
  if(gl_GlobalInvocationID.x >= WIDTH || gl_GlobalInvocationID.y >= HEIGHT)
    return;

  float x = float(gl_GlobalInvocationID.x) / float(WIDTH);
  float y = float(gl_GlobalInvocationID.y) / float(HEIGHT);
  
  imageStore(outputBuffer, ivec2(gl_GlobalInvocationID.xy), vec4(x, y, 0.0f, 1.0f));
})";
        shaderc::Compiler compiler;
        shaderc::CompileOptions options;
        options.SetOptimizationLevel(shaderc_optimization_level_performance);
        shaderc::SpvCompilationResult spv = compiler.CompileGlslToSpv(
            computeShaderGlsl,
            shaderc_glsl_compute_shader,
            "shader",
            options);
        if (spv.GetCompilationStatus() != shaderc_compilation_status_success) {
            std::cerr << spv.GetErrorMessage();
        }
        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.pCode = spv.cbegin();
        createInfo.codeSize = std::distance(spv.cbegin(), spv.cend()) * sizeof(uint32_t);
        VK_CHECK_RESULT(vkCreateShaderModule(device, &createInfo, nullptr, &computeShaderModule));

        VkPipelineShaderStageCreateInfo shaderStageCreateInfo = {};
        shaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        shaderStageCreateInfo.module = computeShaderModule;
        shaderStageCreateInfo.pName = "main";

        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
        pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCreateInfo.setLayoutCount = 1;
        pipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout;
        VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout));

        VkComputePipelineCreateInfo pipelineCreateInfo = {};
        pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        pipelineCreateInfo.stage = shaderStageCreateInfo;
        pipelineCreateInfo.layout = pipelineLayout;
        VK_CHECK_RESULT(vkCreateComputePipelines(
            device,
            VK_NULL_HANDLE,
            1,
            &pipelineCreateInfo,
            nullptr,
            &pipeline));
    }

    void createCommandBuffer()
    {
        VkCommandPoolCreateInfo commandPoolCreateInfo = {};
        commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndex;
        VK_CHECK_RESULT(vkCreateCommandPool(device, &commandPoolCreateInfo, nullptr, &commandPool));

        VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
        commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferAllocateInfo.commandPool = commandPool;
        commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferAllocateInfo.commandBufferCount = 1;
        VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &commandBuffer));
    }

    void render()
    {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0;
        VK_CHECK_RESULT(vkBeginCommandBuffer(commandBuffer, &beginInfo));

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);

        vkCmdDispatch(commandBuffer, (uint32_t)ceil(width / float(WORKGROUP_SIZE)), (uint32_t)ceil(height / float(WORKGROUP_SIZE)), 1);
        VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer));

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        VkFence fence;
        VkFenceCreateInfo fenceCreateInfo = {};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.flags = 0;
        VK_CHECK_RESULT(vkCreateFence(device, &fenceCreateInfo, nullptr, &fence));
        VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, fence));
        VK_CHECK_RESULT(vkWaitForFences(device, 1, &fence, VK_TRUE, 100000000000));
        vkDestroyFence(device, fence, nullptr);
    }

    void cleanup()
    {
        if (enableValidationLayers) {
            DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        }
        vkDestroyImageView(device, sharedOutputView, nullptr);
        vkDestroyImage(device, sharedOutput, nullptr);
        vkFreeMemory(device, sharedOutputMemory, nullptr);
        vkDestroyShaderModule(device, computeShaderModule, nullptr);
        vkDestroyDescriptorPool(device, descriptorPool, nullptr);
        vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
        vkDestroyPipeline(device, pipeline, nullptr);
        vkDestroyCommandPool(device, commandPool, nullptr);
        vkDestroyDevice(device, nullptr);
        vkDestroyInstance(instance, nullptr);
    }
};