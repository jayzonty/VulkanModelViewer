#include "Graphics/Vulkan/VulkanBuffer.hpp"

#include "Graphics/Vulkan/VulkanContext.hpp"

#include <vulkan/vulkan_core.h>

#include <iostream>

/**
 * @brief Constructor
 */
VulkanBuffer::VulkanBuffer()
{
}

/**
 * @brief Destructor
 */
VulkanBuffer::~VulkanBuffer()
{
}

/**
 * @brief Creates the Vulkan buffer given the provided information.
 * @param[in] bufferSize Buffer size
 * @param[in] usageFlags Vulkan flags describing how the buffer will be used
 * @param[in] memoryProperties Vulkan memory properties on how the buffer should be allocated in memory
 * @return Returns true if the creation was successful. Returns false otherwise.
 */
bool VulkanBuffer::Create(VkDeviceSize bufferSize, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryProperties)
{
    VkBufferCreateInfo vertexBufferInfo = {};
    vertexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vertexBufferInfo.size = bufferSize;
    vertexBufferInfo.usage = usageFlags;
    vertexBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // TODO: Expose as a parameter?

    if (vkCreateBuffer(VulkanContext::GetLogicalDevice(), &vertexBufferInfo, nullptr, &m_vkBuffer) != VK_SUCCESS)
    {
        std::cout << "Failed to create buffer!" << std::endl;
        return false;
    }

    // --- Allocate memory for the buffer ---

    // Check memory requirements for the buffer
    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(VulkanContext::GetLogicalDevice(), m_vkBuffer, &memoryRequirements);

    VkMemoryAllocateInfo memoryAllocateInfo = {};
    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.allocationSize = memoryRequirements.size;
    if (!FindSuitableMemoryTypeIndex(memoryRequirements.memoryTypeBits, memoryProperties, memoryAllocateInfo.memoryTypeIndex))
    {
        std::cout << "Failed to find a suitable memory type for the buffer!" << std::endl;
        return false;
    }

    if (vkAllocateMemory(VulkanContext::GetLogicalDevice(), &memoryAllocateInfo, nullptr, &m_vkMemory) != VK_SUCCESS)
    {
        std::cout << "Failed to allocate memory for the buffer!" << std::endl;
        return false;
    }

    // --- Bind the buffer to the memory ---
    vkBindBufferMemory(VulkanContext::GetLogicalDevice(), m_vkBuffer, m_vkMemory, 0);

    return true;
}

/**
 * @brief Maps GPU memory allocated for this buffer to a memory location in RAM
 * @param[in] size Buffer size
 * @return Returns a pointer to the RAM memory that is mapped to the GPU memory for this buffer.
 */
void* VulkanBuffer::MapMemory(VkDeviceSize offset, VkDeviceSize size)
{
    void* mem;
    vkMapMemory(VulkanContext::GetLogicalDevice(), m_vkMemory, offset, size, 0, &mem);
    return mem;
}

/**
 * @brief Unmaps the memory location in RAM that was mapped to the GPU memory for this buffer
 */
void VulkanBuffer::UnmapMemory()
{
    vkUnmapMemory(VulkanContext::GetLogicalDevice(), m_vkMemory);
}

/**
 * @brief Cleans up all resources used by this buffer.
 */
void VulkanBuffer::Cleanup()
{
    if (m_vkBuffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(VulkanContext::GetLogicalDevice(), m_vkBuffer, nullptr);
        m_vkBuffer = VK_NULL_HANDLE;
    }

    if (m_vkMemory != VK_NULL_HANDLE)
    {
        vkFreeMemory(VulkanContext::GetLogicalDevice(), m_vkMemory, nullptr);
        m_vkMemory = VK_NULL_HANDLE;
    }
}

/**
 * @brief Gets the native Vulkan handle for this buffer
 */
VkBuffer VulkanBuffer::GetHandle()
{
    return m_vkBuffer;
}

/**
 * @brief Find the index of a suitable memory type given the requirements
 * @param[in] memoryTypeBits Flag containing the supported memory types
 * @param[in] requiredProperties Flag containing the required memory properties
 * @param[out] outMemoryTypeIndex If a suitable memory type is found, this is where the index of the memory type will be placed
 * @return Returns true if a suitable memory type has been found. Returns false otherwise.
 */
bool VulkanBuffer::FindSuitableMemoryTypeIndex(uint32_t memoryTypeBits, VkMemoryPropertyFlags requiredProperties, uint32_t& outMemoryTypeIndex)
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
