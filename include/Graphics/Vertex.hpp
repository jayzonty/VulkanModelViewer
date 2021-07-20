#pragma once

#include <vulkan/vulkan.hpp>

#include <glm/glm.hpp>

#include <array>

namespace Engine
{
    /**
     * Vertex struct
     */
    struct Vertex
    {
        /**
         * Vertex position
         */
        glm::vec3 position;

        /**
         * Vertex color
         */
        glm::vec3 color;

        /**
         * Vertex UV coordinates
         */
        glm::vec2 uv;

        static VkVertexInputBindingDescription GetBindingDescription()
        {
            VkVertexInputBindingDescription bindingDescription;
            bindingDescription.binding = 0;
            bindingDescription.stride = sizeof(Vertex);
            bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            return bindingDescription;
        };

        static std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescription()
        {
            std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = {};

            // Position
            attributeDescriptions[0].binding = 0;
            attributeDescriptions[0].location = 0;
            attributeDescriptions[0].offset = offsetof(Vertex, position);
            attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT; // Three 32-bit signed floats

            attributeDescriptions[1].binding = 0;
            attributeDescriptions[1].location = 1;
            attributeDescriptions[1].offset = offsetof(Vertex, color);
            attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT; // Three 32-bit signed floats

            attributeDescriptions[2].binding = 0;
            attributeDescriptions[2].location = 2;
            attributeDescriptions[2].offset = offsetof(Vertex, uv);
            attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT; // Two 32-bit signed floats

            return attributeDescriptions;
        }
    };
}
