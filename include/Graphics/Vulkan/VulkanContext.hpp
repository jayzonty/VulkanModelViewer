#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.h>

#include <optional>
#include <vector>

class VulkanContext
{
public:
    // Delete copy constructor and copy operator
    VulkanContext(const VulkanContext&) = delete;
    void operator=(const VulkanContext&) = delete;

    /**
     * @brief Destructor
     */
    ~VulkanContext();

    /**
     * @brief Initializes the Vulkan manager.
     * @param[in] window GLFW window
     * @return Returns true if the initialization was successful. Returns false otherwise.
     */
    static bool Initialize(GLFWwindow* window);

    /**
     * @brief Cleans up the resources used by the Vulkan manager.
     */
    static void Cleanup();

    /**
     * @brief Gets Vulkan instance.
     * @return Returns the vulkan instance.
     */
    static VkInstance GetVulkanInstance();

    /**
     * @brief Gets Vulkan surface.
     * @return Returns the vulkan surface.
     */
    static VkSurfaceKHR GetVulkanSurface();

    /**
     * @brief Gets the current Vulkan physical device in use.
     * @return Returns the current Vulkan physical device in use.
     */
    static VkPhysicalDevice GetPhysicalDevice();

    /**
     * @brief Gets the Vulkan logical device that represents the physical device in use.
     * @return Returns the Vulkan logical device.
     */
    static VkDevice GetLogicalDevice();

    /**
     * @brief Gets the Vulkan graphics queue.
     * @return Returns the Vulkan graphics queue.
     */
    static VkQueue GetGraphicsQueue();

    /**
     * @brief Gets the Vulkan present queue.
     * @return Returns the Vulkan present queue.
     */
    static VkQueue GetPresentQueue();

    /**
     * @brief Gets the index of the graphics queue family.
     * @return Returns the index of the graphics queue family.
     */
    static uint32_t GetGraphicsQueueIndex();

    /**
     * @brief Gets the index of the present queue family.
     * @return Returns the index of the present queue family.
     */
    static uint32_t GetPresentQueueIndex();

    /**
     * @brief Gets the default command pool.
     * @return Returns the default command pool.
     */
    static VkCommandPool GetDefaultCommandPool();

private:
    /**
     * Struct containing the indices for each queue type
     */
    struct QueueFamilyIndices
    {
        /**
         * Graphics queue family index
         */
        std::optional<uint32_t> graphicsQueueFamilyIndex;

        /**
         * Present queue family index
         */
        std::optional<uint32_t> presentQueueFamilyIndex;
    };

private:
    /**
     * Vulkan instance
     */
    VkInstance m_vkInstance;

    /**
     * Vulkan surface
     */
    VkSurfaceKHR m_vkSurface;

    /**
     * Vulkan physical device
     */
    VkPhysicalDevice m_vkPhysicalDevice;

    /**
     * Vulkan logical device
     */
    VkDevice m_vkLogicalDevice;

    /**
     * Vulkan queue family indices
     */
    QueueFamilyIndices m_queueFamilyIndices;

    /**
     * Vulkan graphics queue
     */
    VkQueue m_vkGraphicsQueue;

    /**
     * Vulkan present queue
     */
    VkQueue m_vkPresentQueue;

    /**
     * Vulkan default command pool
     */
    VkCommandPool m_vkDefaultCommandPool;

private:
    /**
     * @brief Constructor
     */
    VulkanContext();

    /**
     * @brief Gets the singleton instance for this class.
     * @return Returns the singleton instance for this class.
     */
    static VulkanContext& GetSingletonInstance();

    /**
     * @brief Initialization code implemented as a member function.
     * @param[in] window GLFW window
     * @return Returns true if the initialization was successful. Returns false otherwise.
     */
    bool InitInternal(GLFWwindow* window);

    /**
     * @brief Cleanup code implemented as a member function.
     */
    void CleanupInternal();

    /**
     * @brief Gets the first graphics card that is suitable for our application.
     * @param[in] instance Vulkan instance
     * @param[in] requiredExtensions Required extensions if any
     * @return Returns the handle to the graphics card that we found suitable. If a suitable
     * device was not found, returns VK_NULL_HANDLE.
     */
    VkPhysicalDevice GetFirstSuitablePhysicalDevice(const VkInstance& instance, const std::vector<const char*>& requiredExtensions);

    /**
     * @brief Checks whether the physical device supports all the provided extensions identified by
     * the extension names.
     * @param[in] physicalDevice Physical device
     * @param[in] extensionNames List of extension names to check support
     * @return Returns true if all the provided extensions are supported. Returns false otherwise.
     */
    bool CheckDeviceExtensionSupport(VkPhysicalDevice physicalDevice, const std::vector<const char*>& extensionNames);

    /**
     * @brief Gets the indices of each queue type in the physical device's queue family.
     * @param[in] physicalDevice Physical device
     * @param[in] surface Surface
     * @return Returns a QueueFamilyIndices struct which contains the indices for each queue type.
     */
    QueueFamilyIndices GetQueueFamilyIndices(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
};
