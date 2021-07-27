#pragma once

#include "Graphics/Mesh.hpp"

#include <assimp/scene.h>

#include <string>
#include <vector>

class Model
{
public:
    /**
     * @brief Constructor
     */
    Model();

    /**
     * @brief Destructor
     */
    ~Model();

    /**
     * @brief Loads the 3D model located in the specified file path.
     * @param[in] modelFilePath Model file path
     * @return Returns true if the operation was successful. Returns false otherwise.
     */
    bool Load(const std::string& modelFilePath);

    /**
     * @brief Gets all the meshes in the model.
     * @return Model meshes
     */
    const std::vector<Mesh*>& GetMeshes() const;

    /**
     * @brief Gets the total number of vertices in the model.
     * @return Total vertex count
     */
    uint32_t GetTotalVertexCount() const;

    /**
     * @brief Gets the total number of triangles in the model.
     * @return Total triangle count
     */
    uint32_t GetTotalTriangleCount() const;

private:
    /**
     * List of meshes in the model
     */
    std::vector<Mesh*> m_meshes;

private:
    /**
     * @brief Processes an Assimp node.
     * @param[in] node Assimp node
     * @param[in] scene Assimp scene
     */
    void ProcessNode(aiNode* node, const aiScene* scene);

    /**
     * @brief Processes an Assimp mesh and transforms it to our own Mesh class.
     * @param[in] mesh Assimp mesh
     * @param[in] scene Assimp scene
     * @param[out] outMesh Transformed mesh data
     */
    void ProcessMesh(aiMesh* mesh, const aiScene* scene, Mesh* outMesh);

    /**
     * @brief Cleans up resources.
     */
    void Cleanup();
};
