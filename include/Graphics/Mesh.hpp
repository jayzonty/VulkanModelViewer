#pragma once

#include "Graphics/Vertex.hpp"

#include <glm/glm.hpp>

#include <cstdint>
#include <string>
#include <vector>

/**
 * Struct containing mesh data
 */
struct Mesh
{
    /**
     * Mesh vertices
     */
    std::vector<Vertex> vertices;

    /**
     * Mesh indices
     */
    std::vector<uint32_t> indices;

    /**
     * File paths to the mesh's diffuse maps
     */
    std::vector<std::string> diffuseMapFilePaths;
};
