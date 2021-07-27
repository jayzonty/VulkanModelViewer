#include "Graphics/Vulkan/VulkanContext.hpp"

#include <iostream>
#include <set>

/**
 * @brief Destructor
 */
VulkanContext::~VulkanContext()
{
}

/**
 * @brief Initializes the Vulkan manager.
 * @param[in] window GLFW window
 * @return Returns true if the initialization was successful. Returns false otherwise.
 */
bool VulkanContext::Initialize(GLFWwindow* window)
{
    return GetSingletonInstance().InitInternal(window);
}

/**
 * @brief Cleans up the resources used by the Vulkan manager.
 */
void VulkanContext::Cleanup()
{
    GetSingletonInstance().CleanupInternal();
}

/**
 * @brief Gets Vulkan instance.
 * @return Returns the vulkan instance.
 */
VkInstance VulkanContext::GetVulkanInstance()
{
    return GetSingletonInstance().m_vkInstance;
}

/**
 * @brief Gets Vulkan surface.
 * @return Returns the vulkan surface.
 */
VkSurfaceKHR VulkanContext::GetVulkanSurface()
{
    return GetSingletonInstance().m_vkSurface;
}

/**
 * @brief Gets the current Vulkan physical device in use.
 * @return Returns the current Vulkan physical device in use.
 */
VkPhysicalDevice VulkanContext::GetPhysicalDevice()
{
    return GetSingletonInstance().m_vkPhysicalDevice;
}

/**
 * @brief Gets the Vulkan logical device that represents the physical device in use.
 * @return Returns the Vulkan logical device.
 */
VkDevice VulkanContext::GetLogicalDevice()
{
    return GetSingletonInstance().m_vkLogicalDevice;
}

/**
 * @brief Gets the Vulkan graphics queue.
 * @return Returns the Vulkan graphics queue.
 */
VkQueue VulkanContext::GetGraphicsQueue()
{
    return GetSingletonInstance().m_vkGraphicsQueue;
}

/**
 * @brief Gets the Vulkan present queue.
 * @return Returns the Vulkan present queue.
 */
VkQueue VulkanContext::GetPresentQueue()
{
    return GetSingletonInstance().m_vkPresentQueue;
}

/**
 * @brief Gets the index of the graphics queue family.
 * @return Returns the index of the graphics queue family.
 */
uint32_t VulkanContext::GetGraphicsQueueIndex()
{
    return GetSingletonInstance().m_queueFamilyIndices.graphicsQueueFamilyIndex.value();
}

/**
 * @brief Gets the index of the present queue family.
 * @return Returns the index of the present queue family.
 */
uint32_t VulkanContext::GetPresentQueueIndex()
{
    return GetSingletonInstance().m_queueFamilyIndices.presentQueueFamilyIndex.value();
}

/**
 * @brief Gets the default command pool.
 * @return Returns the default command pool.
 */
VkCommandPool VulkanContext::GetDefaultCommandPool()
{
    return GetSingletonInstance().m_vkDefaultCommandPool;
}

/**
 * @brief Begins a single use command buffer.
 * @return Returns the command buffer that was created.
 */
VkCommandBuffer VulkanContext::BeginSingleUseCommandBuffer()
{
    VkCommandBufferAllocateInfo commandBufferAllocInfo = {};
    commandBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocInfo.commandPool = VulkanContext::GetDefaultCommandPool();
    commandBufferAllocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(GetLogicalDevice(), &commandBufferAllocInfo, &commandBuffer);

    VkCommandBufferBeginInfo commandBufferBeginInfo = {};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);

    return commandBuffer;
}

/**
 * @brief Ends the single use command buffer.
 * @param[in] commandBuffer Command buffer to end
 */
void VulkanContext::EndSingleUseCommandBuffer(VkCommandBuffer commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(GetGraphicsQueue());

    vkFreeCommandBuffers(GetLogicalDevice(), GetDefaultCommandPool(), 1, &commandBuffer);
}

/**
 * @brief Constructor
 */
VulkanContext::VulkanContext()
    : m_vkInstance(VK_NULL_HANDLE)
    , m_vkSurface(VK_NULL_HANDLE)
    , m_vkPhysicalDevice(VK_NULL_HANDLE)
    , m_vkLogicalDevice(VK_NULL_HANDLE)
    , m_queueFamilyIndices()
    , m_vkGraphicsQueue(VK_NULL_HANDLE)
    , m_vkPresentQueue(VK_NULL_HANDLE)
    , m_vkDefaultCommandPool(VK_NULL_HANDLE)
{
}

/**
 * @brief Gets the singleton instance for this class.
 * @return Returns the singleton instance for this class.
 */
VulkanContext& VulkanContext::GetSingletonInstance()
{
    static VulkanContext instance;
    return instance;
}

/**
 * @brief Initialization code implemented as a member function.
 * @param[in] window GLFW window
 * @return Returns true if the initialization was successful. Returns false otherwise.
 */
bool VulkanContext::InitInternal(GLFWwindow* window)
{
    // --- Setup validation layers ---
    std::vector<const char*> validationLayers =
    {
        "VK_LAYER_KHRONOS_validation"
    };

    // --- Initialize vulkan ---
    VkApplicationInfo applicationInfo = {};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pApplicationName = "Vulkan Model Viewer"; // TODO: Make this modifiable from outside
    applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.pEngineName = "No Engine";
    applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &applicationInfo;

    // Get GLFW required extensions for vulkan and include them in the instance creation
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensionNames = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    instanceCreateInfo.enabledExtensionCount = glfwExtensionCount;
    instanceCreateInfo.ppEnabledExtensionNames = glfwExtensionNames;

    // Include validation layers
    // TODO: Have a check whether the validation layer is supported or not
    instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    instanceCreateInfo.ppEnabledLayerNames = validationLayers.data();

    VkResult instanceCreateResult = vkCreateInstance(&instanceCreateInfo, nullptr, &m_vkInstance);
    if (instanceCreateResult != VK_SUCCESS)
    {
        std::cout << "Failed to create Vulkan instance!" << std::endl;
        Cleanup();
        return false;
    }

    // --- Create surface ---
    if (glfwCreateWindowSurface(m_vkInstance, window, nullptr, &m_vkSurface) != VK_SUCCESS)
    {
        std::cout << "Failed to create GLFW window surface!" << std::endl;
        Cleanup();
        return false;
    }

    // --- Select the graphics card to use ---
    // Prepare required physical device extensions for the physical device
    // (To be used when creating the logical device)
    std::vector<const char*> requiredExtensionNames =
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME // To allow the use of gl_BaseInstance in the shader
    };

    m_vkPhysicalDevice = GetFirstSuitablePhysicalDevice(m_vkInstance, requiredExtensionNames);
    if (m_vkPhysicalDevice == VK_NULL_HANDLE)
    {
        std::cout << "Failed to find suitable graphics card!" << std::endl;
        CleanupInternal();
        return false;
    }

    // --- Prepare device queue creation info structs as part of the logical device creation ---
    m_queueFamilyIndices = GetQueueFamilyIndices(m_vkPhysicalDevice, m_vkSurface);

    std::set<uint32_t> uniqueQueueFamilyIndices =
    {
        m_queueFamilyIndices.graphicsQueueFamilyIndex.value(),
        m_queueFamilyIndices.presentQueueFamilyIndex.value()
    };
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfoStructs;
    for (uint32_t queueFamilyIndex : uniqueQueueFamilyIndices)
    {
        queueCreateInfoStructs.emplace_back();

        queueCreateInfoStructs.back().sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfoStructs.back().queueFamilyIndex = queueFamilyIndex;
        queueCreateInfoStructs.back().queueCount = 1;
        float queuePriority = 1.0f;
        queueCreateInfoStructs.back().pQueuePriorities = &queuePriority;
    }

    VkPhysicalDeviceFeatures physicalDeviceFeatures = {};
    physicalDeviceFeatures.samplerAnisotropy = VK_TRUE; // Enable anisotropic filtering

    // --- Create a logical device associated with the physical device ---
    VkDeviceCreateInfo logicalDeviceCreateInfo = {};
    logicalDeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    logicalDeviceCreateInfo.pEnabledFeatures = &physicalDeviceFeatures;
    logicalDeviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfoStructs.size());
    logicalDeviceCreateInfo.pQueueCreateInfos = queueCreateInfoStructs.data();
    logicalDeviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensionNames.size());
    logicalDeviceCreateInfo.ppEnabledExtensionNames = requiredExtensionNames.data();
    logicalDeviceCreateInfo.enabledLayerCount = 0;
    if (vkCreateDevice(m_vkPhysicalDevice, &logicalDeviceCreateInfo, nullptr, &m_vkLogicalDevice) != VK_SUCCESS)
    {
        std::cout << "Failed to create logical device!" << std::endl;
        CleanupInternal();
        return false;
    }

    // --- Get handles to the device queues that we just created ---
    vkGetDeviceQueue(m_vkLogicalDevice, m_queueFamilyIndices.graphicsQueueFamilyIndex.value(), 0, &m_vkGraphicsQueue);
    vkGetDeviceQueue(m_vkLogicalDevice, m_queueFamilyIndices.presentQueueFamilyIndex.value(), 0, &m_vkPresentQueue);

    VkCommandPoolCreateInfo commandPoolInfo = {};
    commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolInfo.queueFamilyIndex = GetGraphicsQueueIndex();
    commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if (vkCreateCommandPool(m_vkLogicalDevice, &commandPoolInfo, nullptr, &m_vkDefaultCommandPool) != VK_SUCCESS)
    {
        std::cout << "Failed to create default command pool!" << std::endl;
        CleanupInternal();
        return false;
    }

    return true;
}

/**
 * @brief Cleanup code implemented as a member function.
 */
void VulkanContext::CleanupInternal()
{
    // Destroy default command pool
    if (m_vkDefaultCommandPool != VK_NULL_HANDLE)
    {
        vkDestroyCommandPool(m_vkLogicalDevice, m_vkDefaultCommandPool, nullptr);
        m_vkDefaultCommandPool = VK_NULL_HANDLE;
    }

    // Destroy logical device
    if (m_vkLogicalDevice != VK_NULL_HANDLE)
    {
        vkDestroyDevice(m_vkLogicalDevice, nullptr);
        m_vkLogicalDevice = VK_NULL_HANDLE;
    }

    // Destroy surface
    if (m_vkSurface != VK_NULL_HANDLE)
    {
        vkDestroySurfaceKHR(m_vkInstance, m_vkSurface, nullptr);
        m_vkSurface = VK_NULL_HANDLE;
    }

    // Destroy Vulkan instance
    if (m_vkInstance != VK_NULL_HANDLE)
    {
        vkDestroyInstance(m_vkInstance, nullptr);
        m_vkInstance = VK_NULL_HANDLE;
    }
}

/**
 * @brief Get the first graphics card that is suitable for our application.
 * @param[in] instance Vulkan instance
 * @param[in] requiredExtensions Required extensions if any
 * @return Handle to the graphics card that we found suitable. If a suitable
 * device was not found, returns VK_NULL_HANDLE.
 */
VkPhysicalDevice VulkanContext::GetFirstSuitablePhysicalDevice(const VkInstance& instance, const std::vector<const char*>& requiredExtensions)
{
    // First query the number of graphics card in the system
    uint32_t physicalDeviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);

    if (physicalDeviceCount > 0)
    {
        std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
        vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data());

        // Go through each physical device, and check whether it is suitable or not.
        // Once we find one, we return the device immediately.
        for (const VkPhysicalDevice& physicalDevice : physicalDevices)
        {
            // Check if the graphics card supported the provided extensions.
            // If not, skip to the next graphics card.
            if (!CheckDeviceExtensionSupport(physicalDevice, requiredExtensions))
            {
                continue;
            }

            // Query physical device properties
            VkPhysicalDeviceProperties physicalDeviceProperties;
            vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

            // Query physical device features
            VkPhysicalDeviceFeatures physicalDeviceFeatures;
            vkGetPhysicalDeviceFeatures(physicalDevice, &physicalDeviceFeatures);

            // Check for anisotropic filtering capability (most GPUs nowadays should support this)
            if (!physicalDeviceFeatures.samplerAnisotropy)
            {
                continue;
            }

            // We can have a scoring system for each physical device and get the highest scoring
            // one (criteria depends on our requirements). But for now, we'll just settle with the
            // first discrete GPU that we find.
            if (physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            {
                QueueFamilyIndices indices = GetQueueFamilyIndices(physicalDevice, m_vkSurface);
                if (indices.graphicsQueueFamilyIndex.has_value() && indices.presentQueueFamilyIndex.has_value())
                {
                    return physicalDevice;
                }
            }
        }
    }

    return VK_NULL_HANDLE;
}

/**
 * @brief Check whether the physical device supports all the provided extensions identified by
 * the extension names.
 * @param[in] physicalDevice Physical device
 * @param[in] extensionNames List of extension names to check support
 * @return Returns true if all the provided extensions are supported. Returns false otherwise.
 */
bool VulkanContext::CheckDeviceExtensionSupport(VkPhysicalDevice physicalDevice, const std::vector<const char*>& extensionNames)
{
    uint32_t numSupportedExtensions = 0;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &numSupportedExtensions, nullptr);

    std::vector<VkExtensionProperties> supportedExtensions(numSupportedExtensions);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &numSupportedExtensions, supportedExtensions.data());

    std::set<std::string> extensionNamesSet(extensionNames.begin(), extensionNames.end());
    for (const VkExtensionProperties& extension : supportedExtensions)
    {
        extensionNamesSet.erase(extension.extensionName);
    }

    return extensionNamesSet.empty();
}

/**
 * @brief Get the indices of each queue type in the physical device's queue family.
 * @param[in] physicalDevice Physical device
 * @return Returns a QueueFamilyIndices struct which contains the indices for each queue type.
 */
VulkanContext::QueueFamilyIndices VulkanContext::GetQueueFamilyIndices(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    QueueFamilyIndices ret = {};

    uint32_t numQueueFamilies = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &numQueueFamilies, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilyProperties(numQueueFamilies);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &numQueueFamilies, queueFamilyProperties.data());

    for (uint32_t i = 0; i < queueFamilyProperties.size(); ++i)
    {
        // Check if the queue family supports graphics capabilities
        if (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            ret.graphicsQueueFamilyIndex = i;
        }

        // Check if the queue family supports presentation capabilities
        VkBool32 presentationSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentationSupport);
        if (presentationSupport)
        {
            ret.presentQueueFamilyIndex = i;
        }

        if (ret.graphicsQueueFamilyIndex.has_value() && ret.presentQueueFamilyIndex.has_value())
        {
            break;
        }
    }

    return ret;
}