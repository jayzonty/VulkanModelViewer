#include "Graphics/Vulkan/VulkanImageView.hpp"

#include "Graphics/Vulkan/VulkanContext.hpp"

#include <iostream>

/**
 * @brief Constructor
 */
VulkanImageView::VulkanImageView()
    : m_vkImageView(VK_NULL_HANDLE)
{
}

/**
 * @brief Destructor
 */
VulkanImageView::~VulkanImageView()
{
}

/**
 * @brief Creates the Vulkan image view.
 * @param[in] image Vulkan image
 * @param[in] format Vulkan image format
 * @param[in] imageAspectFlags Vulkan image aspect flags
 * @return Returns true if the creation was successful. Returns false otherwise.
 */
bool VulkanImageView::Create(VkImage image, VkFormat format, VkImageAspectFlags imageAspectFlags)
{
    VkImageViewCreateInfo imageViewCreateInfo = {};
    imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCreateInfo.image = image;
    imageViewCreateInfo.format = format;
    imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;

    imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    imageViewCreateInfo.subresourceRange.aspectMask = imageAspectFlags;
    imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    imageViewCreateInfo.subresourceRange.levelCount = 1;
    imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    imageViewCreateInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(VulkanContext::GetLogicalDevice(), &imageViewCreateInfo, nullptr, &m_vkImageView) != VK_SUCCESS)
    {
        std::cout << " FAILED!" << std::endl;
        return false;
    }

    return true;
}

/**
 * @brief Cleans up the resources used.
 */
void VulkanImageView::Cleanup()
{
    if (m_vkImageView != VK_NULL_HANDLE)
    {
        vkDestroyImageView(VulkanContext::GetLogicalDevice(), m_vkImageView, nullptr);
        m_vkImageView = VK_NULL_HANDLE;
    }
}

/**
 * @brief Gets the native Vulkan handle for the image view.
 * return Returns the native Vulkan handle for the image view.
 */
VkImageView VulkanImageView::GetHandle()
{
    return m_vkImageView;
}
