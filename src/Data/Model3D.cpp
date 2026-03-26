#include "Model3D.h"

#include <unordered_set>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

bool Model3D::Load(const std::string& path) {
    meshes_.clear();
    error_.clear();

    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(
        path,
        aiProcess_Triangulate
        | aiProcess_GenSmoothNormals
        | aiProcess_FlipUVs
        | aiProcess_JoinIdenticalVertices
    );

    if (scene == nullptr || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || scene->mRootNode == nullptr) {
        error_ = importer.GetErrorString();
        return false;
    }

    ProcessNode(scene->mRootNode, scene);
    return !meshes_.empty();
}

void Model3D::ProcessNode(const aiNode* node, const aiScene* scene) {
    for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
        const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes_.push_back(ProcessMesh(mesh));
        ComputeCurvature(meshes_.back());
    }

    for (unsigned int i = 0; i < node->mNumChildren; ++i) {
        ProcessNode(node->mChildren[i], scene);
    }
}

Model3D::Mesh Model3D::ProcessMesh(const aiMesh* mesh) {
    Mesh result;
    result.vertices.reserve(mesh->mNumVertices);

    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
        Vertex vertex;

        vertex.position = {
            mesh->mVertices[i].x,
            mesh->mVertices[i].y,
            mesh->mVertices[i].z
        };

        if (mesh->mNormals != nullptr) {
            vertex.normal = {
                mesh->mNormals[i].x,
                mesh->mNormals[i].y,
                mesh->mNormals[i].z
            };
        }

        if (mesh->mTextureCoords[0] != nullptr) {
            vertex.uv = {
                mesh->mTextureCoords[0][i].x,
                mesh->mTextureCoords[0][i].y
            };
        }

        result.vertices.push_back(vertex);
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
        const aiFace& face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; ++j) {
            result.indices.push_back(face.mIndices[j]);
        }
    }

    return result;
}

void Model3D::ComputeCurvature(Mesh& mesh) {
    const size_t vertex_count = mesh.vertices.size();
    if (vertex_count == 0) {
        return;
    }

    // Build adjacency: for each vertex, collect connected neighbor indices.
    std::vector<std::unordered_set<uint32_t>> adjacency(vertex_count);
    for (size_t i = 0; i + 2 < mesh.indices.size(); i += 3) {
        uint32_t i0 = mesh.indices[i];
        uint32_t i1 = mesh.indices[i + 1];
        uint32_t i2 = mesh.indices[i + 2];
        adjacency[i0].insert(i1); adjacency[i0].insert(i2);
        adjacency[i1].insert(i0); adjacency[i1].insert(i2);
        adjacency[i2].insert(i0); adjacency[i2].insert(i1);
    }

    // Compute raw curvature per vertex: average of (1 - dot(n_self, n_neighbor)) / 2
    // for all connected neighbors.  Result is in [0, 1]: 0 = flat, 1 = sharp crease.
    std::vector<float> raw_curvature(vertex_count, 0.0f);
    for (size_t v = 0; v < vertex_count; ++v) {
        const auto& neighbors = adjacency[v];
        if (neighbors.empty()) {
            continue;
        }
        const glm::vec3& n_self = mesh.vertices[v].normal;
        float sum = 0.0f;
        for (uint32_t nb : neighbors) {
            float d = glm::dot(n_self, mesh.vertices[nb].normal);
            sum += (1.0f - d) * 0.5f;
        }
        raw_curvature[v] = sum / static_cast<float>(neighbors.size());
    }

    // Smooth: replace each vertex's curvature with the average of its neighbors'
    // raw curvature values.
    for (size_t v = 0; v < vertex_count; ++v) {
        const auto& neighbors = adjacency[v];
        if (neighbors.empty()) {
            mesh.vertices[v].curvature = raw_curvature[v];
            continue;
        }
        float sum = 0.0f;
        for (uint32_t nb : neighbors) {
            sum += raw_curvature[nb];
        }
        mesh.vertices[v].curvature = sum / static_cast<float>(neighbors.size());
    }
}
