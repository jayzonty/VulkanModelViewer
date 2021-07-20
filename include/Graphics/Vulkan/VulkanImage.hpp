#pragma once

#include <vulkan/vulkan.hpp>

#include <string>
#include <vulkan/vulkan_core.h>

/**
 * Vulkan image
 */
class VulkanImage
{
public:
    /**
     * @brief Constructor
     */
    VulkanImage();

    /**
     * @brief Destructor
     */
    ~VulkanImage();

    /**
     * @brief Creates the image given the provided information.
     * @param[in] width Image width
     * @param[in] height Image height
     * @param[in] format Image format
     * @param[in] tiling Image tiling
     * @param[in] tiling Image tiling type
     * @param[in] usageFlags Vulkan flags indicating how the image will be used
     * @param[in] memoryProperties Properties describing how to allocate memory for this image
     * @reutnr Returns true if the creation was successful. Returns false otherwise.
     */
    bool Create(const uint32_t& width, const uint32_t& height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usageFlags, VkMemoryPropertyFlags memoryProperties);

    /**
     * @brief Clean up resources used.
     */
    void Cleanup();

    /**
     * @brief Gets the Vulkan handle for this image.
     * @return Returns the native Vulkan handle for this image.
     */
    VkImage GetHandle();

private:
    /**
     * Vulkan image handle
     */
    VkImage m_vkImage;

    /**
     * Vulkan memory handle
     */
    VkDeviceMemory m_vkMemory;

    /**
     * @brief Find the index of a suitable memory type given the requirements
     * @param[in] memoryTypeBits Flag containing the supported memory types
     * @param[in] requiredProperties Flag containing the required memory properties
     * @param[out] outMemoryTypeIndex If a suitable memory type is found, this is where the index of the memory type will be placed
     * @return Returns true if a suitable memory type has been found. Returns false otherwise.
     */
    bool FindSuitableMemoryTypeIndex(uint32_t memoryTypeBits, VkMemoryPropertyFlags requiredProperties, uint32_t& outMemoryTypeIndex);
};

