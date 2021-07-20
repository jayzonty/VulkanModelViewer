#pragma once

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>

/**
 * Vulkan image view wrapper
 */
class VulkanImageView
{
public:
    /**
     * @brief Constructor
     */
    VulkanImageView();

    /**
     * @brief Destructor
     */
    ~VulkanImageView();

    /**
     * @brief Creates the Vulkan image view.
     * @param[in] image Vulkan image
     * @param[in] format Vulkan image format
     * @param[in] imageAspectFlags Vulkan image aspect flags
     * @return Returns true if the creation was successful. Returns false otherwise.
     */
    bool Create(VkImage image, VkFormat format, VkImageAspectFlags imageAspectFlags);

    /**
     * @brief Cleans up the resources used.
     */
    void Cleanup();

    /**
     * @brief Gets the native Vulkan handle for the image view.
     * return Returns the native Vulkan handle for the image view.
     */
    VkImageView GetHandle();

private:
    /**
     * Vulkan image view handle.
     */
    VkImageView m_vkImageView;
};
