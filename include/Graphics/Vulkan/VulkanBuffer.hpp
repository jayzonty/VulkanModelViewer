#pragma once

#include "Graphics/Vertex.hpp"

#include <vulkan/vulkan.hpp>

#include <vector>

/**
* Vulkan buffer class
*/
class VulkanBuffer
{
public:
    /**
     * @brief Constructor
     */
    VulkanBuffer();

    /**
     * @brief Destructor
     */
    ~VulkanBuffer();

    /**
     * @brief Creates the Vulkan buffer given the provided information.
     * @param[in] bufferSize Buffer size
     * @param[in] usageFlags Vulkan flags describing how the buffer will be used
     * @param[in] memoryProperties Vulkan memory properties on how the buffer should be allocated in memory
     * @return Returns true if the creation was successful. Returns false otherwise.
     */
    bool Create(VkDeviceSize bufferSize, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryProperties);

    /**
     * @brief Maps GPU memory allocated for this buffer to a memory location in RAM
     * @param[in] offset Offset from the start of the memory
     * @param[in] size Buffer size
     * @return Returns a pointer to the RAM memory that is mapped to the GPU memory for this buffer.
     */
    void* MapMemory(VkDeviceSize offset, VkDeviceSize size);

    /**
     * @brief Unmaps the memory location in RAM that was mapped to the GPU memory for this buffer
     */
    void UnmapMemory();

    /**
     * @brief Cleans up all resources used by this buffer.
     */
    void Cleanup();

    /**
     * @brief Gets the native Vulkan handle for this buffer
     */
    VkBuffer GetHandle();

private:
    /**
     * Vulkan buffer
     */
    VkBuffer m_vkBuffer;

    /**
     * Vulkan memory allocated for this buffer
     */
    VkDeviceMemory m_vkMemory;

private:
    /**
     * @brief Find the index of a suitable memory type given the requirements
     * @param[in] memoryTypeBits Flag containing the supported memory types
     * @param[in] requiredProperties Flag containing the required memory properties
     * @param[out] outMemoryTypeIndex If a suitable memory type is found, this is where the index of the memory type will be placed
     * @return Returns true if a suitable memory type has been found. Returns false otherwise.
     */
    bool FindSuitableMemoryTypeIndex(uint32_t memoryTypeBits, VkMemoryPropertyFlags requiredProperties, uint32_t& outMemoryTypeIndex);
};
