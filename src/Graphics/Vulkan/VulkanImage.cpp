#include "Graphics/Vulkan/VulkanImage.hpp"

#include "Graphics/Vulkan/VulkanContext.hpp"
#include <vulkan/vulkan_core.h>

#include <iostream>

/**
 * @brief Constructor
 */
VulkanImage::VulkanImage()
    : m_vkImage(VK_NULL_HANDLE)
    , m_vkMemory(VK_NULL_HANDLE)
{
}

/**
 * @brief Destructor
 */
VulkanImage::~VulkanImage()
{
}

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
bool VulkanImage::Create(const uint32_t& width, const uint32_t& height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usageFlags, VkMemoryPropertyFlags memoryProperties)
{
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usageFlags;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(VulkanContext::GetLogicalDevice(), &imageInfo, nullptr, &m_vkImage) != VK_SUCCESS)
    {
        std::cout << "Failed to create image!" << std::endl;
        return false;
    }

    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(VulkanContext::GetLogicalDevice(), m_vkImage, &memoryRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memoryRequirements.size;
    FindSuitableMemoryTypeIndex(memoryRequirements.memoryTypeBits, memoryProperties, allocInfo.memoryTypeIndex);
    if (vkAllocateMemory(VulkanContext::GetLogicalDevice(), &allocInfo, nullptr, &m_vkMemory) != VK_SUCCESS)
    {
        std::cout << "Failed to allocate memory for the image!" << std::endl;
        return false;
    }

    vkBindImageMemory(VulkanContext::GetLogicalDevice(), m_vkImage, m_vkMemory, 0);

    return true;
}

/**
 * @brief Clean up resources used.
 */
void VulkanImage::Cleanup()
{
    if (m_vkImage != VK_NULL_HANDLE)
    {
        vkDestroyImage(VulkanContext::GetLogicalDevice(), m_vkImage, nullptr);
        m_vkImage = VK_NULL_HANDLE;
    }
    if (m_vkMemory != VK_NULL_HANDLE)
    {
        vkFreeMemory(VulkanContext::GetLogicalDevice(), m_vkMemory, nullptr);
        m_vkMemory = VK_NULL_HANDLE;
    }
}

/**
 * @brief Gets the Vulkan handle for this image.
 * @return Returns the native Vulkan handle for this image.
 */
VkImage VulkanImage::GetHandle()
{
    return m_vkImage;
}

/**
 * @brief Find the index of a suitable memory type given the requirements
 * @param[in] memoryTypeBits Flag containing the supported memory types
 * @param[in] requiredProperties Flag containing the required memory properties
 * @param[out] outMemoryTypeIndex If a suitable memory type is found, this is where the index of the memory type will be placed
 * @return Returns true if a suitable memory type has been found. Returns false otherwise.
 */
bool VulkanImage::FindSuitableMemoryTypeIndex(uint32_t memoryTypeBits, VkMemoryPropertyFlags requiredProperties, uint32_t& outMemoryTypeIndex)
{
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(VulkanContext::GetPhysicalDevice(), &memoryProperties);

    // Go through each bit in the memoryTypeBits and check if the bit is set
    // and the memory type at that particular index supports the required properties.
    // For example, we check if bit 5 in the memoryTypeBits is set (meaning it is supported) and
    // if the memory type at index 5 supports all the flags in the requiredProperties flag
    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
    {
        if ((memoryTypeBits & (1 << i)) 
                && ((memoryProperties.memoryTypes[i].propertyFlags & requiredProperties) == requiredProperties))
        {
            outMemoryTypeIndex = i;
            return true;
        }
    }

    return false;
}

