#include "Graphics/Model.hpp"

#include <assimp/Importer.hpp>
#include <assimp/material.h>
#include <assimp/postprocess.h>

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
    for (size_t i = 0; i < m_meshes.size(); ++i)
    {
        delete m_meshes[i];
    }
    m_meshes.clear();
}

/**
 * @brief Loads the 3D model located in the specified file path.
 * @param[in] modelFilePath Model file path
 * @return Returns true if the operation was successful. Returns false otherwise.
 */
bool Model::Load(const std::string& modelFilePath)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(modelFilePath.c_str(), aiProcess_PreTransformVertices | aiProcess_Triangulate | aiProcess_FlipUVs);

    if ((scene == nullptr) || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || (scene->mRootNode == nullptr))
    {
        std::cerr << "Failed to load model: " << modelFilePath << std::endl;
        return false;
    }

    std::string directory = modelFilePath.substr(0, modelFilePath.find_last_of('/'));
    std::cout << "Model directory: " << directory << std::endl;

    ProcessNode(scene->mRootNode, scene);

    for (size_t i = 0; i < m_meshes.size(); ++i)
    {
        for (size_t j = 0; j < m_meshes[i]->diffuseMapFilePaths.size(); ++j)
        {
            m_meshes[i]->diffuseMapFilePaths[j] = directory + "/" + m_meshes[i]->diffuseMapFilePaths[j];
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