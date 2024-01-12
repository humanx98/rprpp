#include <assert.h>
#include <iostream>
#include <lodepng.h>
#include <shaderc/shaderc.hpp>
#include <vector>
#include <vulkan/vulkan.h>

const int WIDTH = 3200;
const int HEIGHT = 2400;
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

class ComputeApplication {
private:
    typedef union {
        struct {
            float x, y, z, w;
        };
        float data[4];
    } float4;

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
    VkBuffer buffer;
    VkDeviceMemory bufferMemory;
    uint32_t bufferSize;
    VkQueue queue;
    uint32_t queueFamilyIndex;
    std::vector<const char*> enabledLayers;

public:
    void run()
    {
        bufferSize = sizeof(float4) * WIDTH * HEIGHT;
        createInstance();
        setupDebugMessenger();
        findPhysicalDevice();
        createDevice();
        createBuffer();
        createDescriptorSetLayout();
        createDescriptorSet();
        createComputePipeline();
        createCommandBuffer();
        runCommandBuffer();
        saveRenderedImage();
        cleanup();
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
        applicationInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.flags = 0;
        createInfo.pApplicationInfo = &applicationInfo;


        std::vector<const char*> enabledExtensions;
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        if (enableValidationLayers) {
            uint32_t layerCount;
            vkEnumerateInstanceLayerProperties(&layerCount, NULL);
            std::vector<VkLayerProperties> layerProperties(layerCount);
            vkEnumerateInstanceLayerProperties(&layerCount, layerProperties.data());

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
            vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);
            std::vector<VkExtensionProperties> extensionProperties(extensionCount);
            vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, extensionProperties.data());

            bool foundExtension = false;
            for (VkExtensionProperties& prop : extensionProperties) {
                if (strcmp(VK_EXT_DEBUG_UTILS_EXTENSION_NAME, prop.extensionName) == 0) {
                    foundExtension = true;
                    break;
                }
            }

            if (!foundExtension) {
                throw std::runtime_error("Extension VK_EXT_DEBUG_UTILS_EXTENSION_NAME not supported\n");
            }

            enabledExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
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
        VK_CHECK_RESULT(vkCreateInstance(&createInfo, NULL, &instance));
    }

    void findPhysicalDevice()
    {
        uint32_t deviceCount;
        vkEnumeratePhysicalDevices(instance, &deviceCount, NULL);
        if (deviceCount == 0) {
            throw std::runtime_error("could not find a device with vulkan support");
        }
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        for (VkPhysicalDevice device : devices) {
            if (true) { // no feature checks right now, so just accept
                physicalDevice = device;
                break;
            }
        }
    }

    uint32_t getComputeQueueFamilyIndex()
    {
        uint32_t queueFamilyCount;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, NULL);
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
        deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
        deviceCreateInfo.queueCreateInfoCount = 1;
        deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

        VK_CHECK_RESULT(vkCreateDevice(physicalDevice, &deviceCreateInfo, NULL, &device));
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

    void createBuffer()
    {
        VkBufferCreateInfo bufferCreateInfo = {};
        bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferCreateInfo.size = bufferSize;
        bufferCreateInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        VK_CHECK_RESULT(vkCreateBuffer(device, &bufferCreateInfo, NULL, &buffer));

        VkMemoryRequirements memoryRequirements;
        vkGetBufferMemoryRequirements(device, buffer, &memoryRequirements);

        VkMemoryAllocateInfo allocateInfo = {};
        allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocateInfo.allocationSize = memoryRequirements.size;
        allocateInfo.memoryTypeIndex = findMemoryType(
            memoryRequirements.memoryTypeBits,
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

        VK_CHECK_RESULT(vkAllocateMemory(device, &allocateInfo, NULL, &bufferMemory));
        VK_CHECK_RESULT(vkBindBufferMemory(device, buffer, bufferMemory, 0));
    }

    void createDescriptorSetLayout()
    {
        VkDescriptorSetLayoutBinding descriptorSetLayoutBinding = {};
        descriptorSetLayoutBinding.binding = 0;
        descriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorSetLayoutBinding.descriptorCount = 1;
        descriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};
        descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayoutCreateInfo.bindingCount = 1;
        descriptorSetLayoutCreateInfo.pBindings = &descriptorSetLayoutBinding;
        VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCreateInfo, NULL, &descriptorSetLayout));
    }

    void createDescriptorSet()
    {
        VkDescriptorPoolSize descriptorPoolSize = {};
        descriptorPoolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorPoolSize.descriptorCount = 1;

        VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
        descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptorPoolCreateInfo.maxSets = 1;
        descriptorPoolCreateInfo.poolSizeCount = 1;
        descriptorPoolCreateInfo.pPoolSizes = &descriptorPoolSize;
        VK_CHECK_RESULT(vkCreateDescriptorPool(device, &descriptorPoolCreateInfo, NULL, &descriptorPool));

        VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
        descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        descriptorSetAllocateInfo.descriptorPool = descriptorPool;
        descriptorSetAllocateInfo.descriptorSetCount = 1;
        descriptorSetAllocateInfo.pSetLayouts = &descriptorSetLayout;
        VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, &descriptorSet));

        VkDescriptorBufferInfo descriptorBufferInfo = {};
        descriptorBufferInfo.buffer = buffer;
        descriptorBufferInfo.offset = 0;
        descriptorBufferInfo.range = bufferSize;

        VkWriteDescriptorSet writeDescriptorSet = {};
        writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet.dstSet = descriptorSet;
        writeDescriptorSet.dstBinding = 0;
        writeDescriptorSet.descriptorCount = 1;
        writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        writeDescriptorSet.pBufferInfo = &descriptorBufferInfo;
        vkUpdateDescriptorSets(device, 1, &writeDescriptorSet, 0, NULL);
    }

    void createComputePipeline()
    {
        std::string computeShaderGlsl = R"(#version 450
#extension GL_ARB_separate_shader_objects : enable

#define WIDTH 3200
#define HEIGHT 2400
#define WORKGROUP_SIZE 32
layout (local_size_x = WORKGROUP_SIZE, local_size_y = WORKGROUP_SIZE, local_size_z = 1 ) in;

struct Pixel{
  vec4 value;
};

layout(std140, binding = 0) buffer buf
{
   Pixel imageData[];
};

void main() {

  /*
  In order to fit the work into workgroups, some unnecessary threads are launched.
  We terminate those threads here. 
  */
  if(gl_GlobalInvocationID.x >= WIDTH || gl_GlobalInvocationID.y >= HEIGHT)
    return;

  float x = float(gl_GlobalInvocationID.x) / float(WIDTH);
  float y = float(gl_GlobalInvocationID.y) / float(HEIGHT);

  /*
  What follows is code for rendering the mandelbrot set. 
  */
  vec2 uv = vec2(x,y);
  float n = 0.0;
  vec2 c = vec2(-.445, 0.0) +  (uv - 0.5)*(2.0+ 1.7*0.2  ), 
  z = vec2(0.0);
  const int M =128;
  for (int i = 0; i<M; i++)
  {
    z = vec2(z.x*z.x - z.y*z.y, 2.*z.x*z.y) + c;
    if (dot(z, z) > 2) break;
    n++;
  }
          
  // we use a simple cosine palette to determine color:
  // http://iquilezles.org/www/articles/palettes/palettes.htm         
  float t = float(n) / float(M);
  vec3 d = vec3(0.3, 0.3 ,0.5);
  vec3 e = vec3(-0.2, -0.3 ,-0.5);
  vec3 f = vec3(2.1, 2.0, 3.0);
  vec3 g = vec3(0.0, 0.1, 0.0);
  vec4 color = vec4( d + e*cos( 6.28318*(f*t+g) ) ,1.0);
          
  // store the rendered mandelbrot set into a storage buffer:
  imageData[WIDTH * gl_GlobalInvocationID.y + gl_GlobalInvocationID.x].value = color;
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
        VK_CHECK_RESULT(vkCreateShaderModule(device, &createInfo, NULL, &computeShaderModule));

        VkPipelineShaderStageCreateInfo shaderStageCreateInfo = {};
        shaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        shaderStageCreateInfo.module = computeShaderModule;
        shaderStageCreateInfo.pName = "main";

        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
        pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCreateInfo.setLayoutCount = 1;
        pipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout;
        VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, NULL, &pipelineLayout));

        VkComputePipelineCreateInfo pipelineCreateInfo = {};
        pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        pipelineCreateInfo.stage = shaderStageCreateInfo;
        pipelineCreateInfo.layout = pipelineLayout;
        VK_CHECK_RESULT(vkCreateComputePipelines(
            device,
            VK_NULL_HANDLE,
            1,
            &pipelineCreateInfo,
            NULL,
            &pipeline));
    }

    void createCommandBuffer()
    {
        VkCommandPoolCreateInfo commandPoolCreateInfo = {};
        commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        commandPoolCreateInfo.flags = 0;
        commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndex;
        VK_CHECK_RESULT(vkCreateCommandPool(device, &commandPoolCreateInfo, NULL, &commandPool));

        VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
        commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferAllocateInfo.commandPool = commandPool;
        commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferAllocateInfo.commandBufferCount = 1;
        VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &commandBuffer));

        {
            VkCommandBufferBeginInfo beginInfo = {};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            VK_CHECK_RESULT(vkBeginCommandBuffer(commandBuffer, &beginInfo));

            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1, &descriptorSet, 0, NULL);

            vkCmdDispatch(commandBuffer, (uint32_t)ceil(WIDTH / float(WORKGROUP_SIZE)), (uint32_t)ceil(HEIGHT / float(WORKGROUP_SIZE)), 1);
            VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer));
        }
    }

    void runCommandBuffer()
    {
        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        VkFence fence;
        VkFenceCreateInfo fenceCreateInfo = {};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.flags = 0;
        VK_CHECK_RESULT(vkCreateFence(device, &fenceCreateInfo, NULL, &fence));
        VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, fence));
        VK_CHECK_RESULT(vkWaitForFences(device, 1, &fence, VK_TRUE, 100000000000));
        vkDestroyFence(device, fence, NULL);
    }

    void saveRenderedImage()
    {
        void* mappedMemory = NULL;
        vkMapMemory(device, bufferMemory, 0, bufferSize, 0, &mappedMemory);
        float4* pmappedMemory = (float4*)mappedMemory;

        std::vector<unsigned char> image;
        image.reserve(WIDTH * HEIGHT * 4);
        for (int i = 0; i < WIDTH * HEIGHT; i += 1) {
            image.push_back((unsigned char)(255.0f * (pmappedMemory[i].x)));
            image.push_back((unsigned char)(255.0f * (pmappedMemory[i].y)));
            image.push_back((unsigned char)(255.0f * (pmappedMemory[i].z)));
            image.push_back((unsigned char)(255.0f * (pmappedMemory[i].w)));
        }
        vkUnmapMemory(device, bufferMemory);

        unsigned error = lodepng::encode("mandelbrot.png", image, WIDTH, HEIGHT);
        if (error)
            printf("encoder error %d: %s", error, lodepng_error_text(error));
    }

    void cleanup()
    {
        if (enableValidationLayers) {
            DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        }
        vkFreeMemory(device, bufferMemory, NULL);
        vkDestroyBuffer(device, buffer, NULL);
        vkDestroyShaderModule(device, computeShaderModule, NULL);
        vkDestroyDescriptorPool(device, descriptorPool, NULL);
        vkDestroyDescriptorSetLayout(device, descriptorSetLayout, NULL);
        vkDestroyPipelineLayout(device, pipelineLayout, NULL);
        vkDestroyPipeline(device, pipeline, NULL);
        vkDestroyCommandPool(device, commandPool, NULL);
        vkDestroyDevice(device, NULL);
        vkDestroyInstance(instance, NULL);
    }
};

int main(int argc, const char* argv[])
{
    std::cout << "VkDx11Interop App started..." << std::endl;
    ComputeApplication app;

    try {
        app.run();
    } catch (const std::runtime_error& e) {
        printf("%s\n", e.what());
        return EXIT_FAILURE;
    }
    std::cout << "VkDx11Interop App finished..." << std::endl;
    return 0;
}