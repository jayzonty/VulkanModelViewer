#include "Graphics/Model.hpp"

#include <assimp/Importer.hpp>
#include <assimp/material.h>
#include <assimp/postprocess.h>

#include <filesystem>
#include <iostream>

/**
 * @brief Constructor
 */
Model::Model()
    : m_meshes()
{
}

/**
 * @brief Destructor
 */
Model::~Model()
{
    Cleanup();
}

/**
 * @brief Loads the 3D model located in the specified file path.
 * @param[in] modelFilePath Model file path
 * @return Returns true if the operation was successful. Returns false otherwise.
 */
bool Model::Load(const std::string& modelFilePath)
{
    Cleanup();
    
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(modelFilePath.c_str(), aiProcess_PreTransformVertices | aiProcess_Triangulate | aiProcess_FlipUVs);

    if ((scene == nullptr) || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || (scene->mRootNode == nullptr))
    {
        std::cerr << "Failed to load model: " << modelFilePath << std::endl;
        return false;
    }

    std::filesystem::path modelDirPath = modelFilePath;
    modelDirPath = modelDirPath.make_preferred();
    modelDirPath = modelDirPath.remove_filename();

    std::cout << "Model directory: " << modelDirPath << std::endl;

    ProcessNode(scene->mRootNode, scene);

    for (size_t i = 0; i < m_meshes.size(); ++i)
    {
        for (size_t j = 0; j < m_meshes[i]->diffuseMapFilePaths.size(); ++j)
        {
            std::filesystem::path diffuseMapFilePath = modelDirPath / m_meshes[i]->diffuseMapFilePaths[j];
            m_meshes[i]->diffuseMapFilePaths[j] = diffuseMapFilePath.string();
            std::cout << "Texture " << j << ": " << m_meshes[i]->diffuseMapFilePaths[j];
        }
        std::cout << "Mesh " << i << " diffuse map count: " << m_meshes[i]->diffuseMapFilePaths.size() << std::endl;
    }

    return true;
}

/**
 * @brief Gets all the meshes in the model.
 * @return Model meshes
 */
const std::vector<Mesh*>& Model::GetMeshes() const
{
    return m_meshes;
}

/**
 * @brief Gets the total number of vertices in the model.
 * @return Total vertex count
 */
uint32_t Model::GetTotalVertexCount() const
{
    uint32_t ret = 0;
    for (size_t i = 0; i < m_meshes.size(); ++i)
    {
        ret += m_meshes[i]->vertices.size();
    }
    return ret;
}

/**
 * @brief Gets the total number of triangles in the model.
 * @return Total triangle count
 */
uint32_t Model::GetTotalTriangleCount() const
{
    uint32_t ret = 0;
    for (size_t i = 0; i < m_meshes.size(); ++i)
    {
        ret += m_meshes[i]->indices.size();
    }
    ret /= 3;
    return ret;
}

/**
 * @brief Processes an Assimp node.
 * @param[in] node Assimp node
 * @param[in] scene Assimp scene
 */
void Model::ProcessNode(aiNode* node, const aiScene* scene)
{
    for (unsigned int i = 0; i < node->mNumMeshes; ++i)
    {
        unsigned int nodeMeshIndex = node->mMeshes[i];
        aiMesh* assimpMesh = scene->mMeshes[nodeMeshIndex];

        Mesh* mesh = new Mesh();
        ProcessMesh(assimpMesh, scene, mesh);
        m_meshes.push_back(mesh);
    }

    // Recurse through child nodes
    for (unsigned int i = 0; i < node->mNumChildren; ++i)
    {
        ProcessNode(node->mChildren[i], scene);
    }
}

/**
 * @brief Processes an Assimp mesh and transforms it to our own Mesh class.
 * @param[in] mesh Assimp mesh
 * @param[in] scene Assimp scene
 * @param[out] outMesh Transformed mesh data
 */
void Model::ProcessMesh(aiMesh* mesh, const aiScene* scene, Mesh* outMesh)
{
    outMesh->vertices.clear();
    for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
    {
        outMesh->vertices.emplace_back();

        Vertex& vertex = outMesh->vertices.back();
        vertex.position = glm::vec3(
            mesh->mVertices[i].x, 
            mesh->mVertices[i].y, 
            mesh->mVertices[i].z
        );

        vertex.color = glm::vec4(
            1.0f, 1.0f, 1.0f, 1.0f
        );

        if (mesh->mTextureCoords[0] != nullptr)
        {
            vertex.uv = glm::vec2(
                mesh->mTextureCoords[0][i].x,
                mesh->mTextureCoords[0][i].y
            );
        }
    }

    outMesh->indices.clear();
    for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; ++j)
        {
            outMesh->indices.push_back(static_cast<uint32_t>(face.mIndices[j]));
        }
    }

    outMesh->diffuseMapFilePaths.clear();
    if (mesh->mMaterialIndex >= 0)
    {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        // Diffuse maps
        unsigned int numDiffuseMaps = material->GetTextureCount(aiTextureType_DIFFUSE);
        for (unsigned int i = 0; i < numDiffuseMaps; ++i)
        {
            aiString textureFilePath;
            material->GetTexture(aiTextureType_DIFFUSE, i, &textureFilePath);
            outMesh->diffuseMapFilePaths.push_back(textureFilePath.C_Str());
        }
    }
}

/**
 * @brief Cleans up resources.
 */
void Model::Cleanup()
{
    for (size_t i = 0; i < m_meshes.size(); ++i)
    {
        delete m_meshes[i];
    }
    m_meshes.clear();
}
