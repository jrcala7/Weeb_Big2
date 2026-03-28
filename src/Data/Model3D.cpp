#include "Model3D.h"

#include <unordered_set>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/material.h>

void Model3D::Unload() {
    for (auto& mesh : meshes_) {
        if (bgfx::isValid(mesh.vbh)) {
            bgfx::destroy(mesh.vbh);
            mesh.vbh = BGFX_INVALID_HANDLE;
        }
        if (bgfx::isValid(mesh.ibh)) {
            bgfx::destroy(mesh.ibh);
            mesh.ibh = BGFX_INVALID_HANDLE;
        }
    }
    meshes_.clear();
    error_.clear();
    directory_.clear();
}

bool Model3D::Load(const std::string& path) {
    meshes_.clear();
    error_.clear();

    auto last_sep = path.find_last_of("/\\");
    directory_ = (last_sep != std::string::npos) ? path.substr(0, last_sep + 1) : "";

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

    // Create persistent GPU vertex/index buffers for each loaded mesh.
    bgfx::VertexLayout layout = GetVertexLayout();
    for (auto& mesh : meshes_) {
        if (mesh.vertices.empty() || mesh.indices.empty()) {
            continue;
        }
        const bgfx::Memory* vb_mem = bgfx::copy(
            mesh.vertices.data(),
            static_cast<uint32_t>(mesh.vertices.size() * sizeof(Vertex))
        );
        mesh.vbh = bgfx::createVertexBuffer(vb_mem, layout);

        const bgfx::Memory* ib_mem = bgfx::copy(
            mesh.indices.data(),
            static_cast<uint32_t>(mesh.indices.size() * sizeof(uint32_t))
        );
        mesh.ibh = bgfx::createIndexBuffer(ib_mem, BGFX_BUFFER_INDEX32);
    }

    return !meshes_.empty();
}

void Model3D::ProcessNode(const aiNode* node, const aiScene* scene) {
    for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
        const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        const aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        meshes_.push_back(ProcessMesh(mesh, material, directory_));
        ComputeCurvature(meshes_.back());
        ComputeSmoothNormals(meshes_.back());
        ComputeTangentBitangent(meshes_.back());
    }

    for (unsigned int i = 0; i < node->mNumChildren; ++i) {
        ProcessNode(node->mChildren[i], scene);
    }
}

Model3D::Mesh Model3D::ProcessMesh(const aiMesh* mesh, const aiMaterial* material, const std::string& directory) {
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

    // Load the base color (diffuse) texture from the material, if any.
    if (material != nullptr) {
        aiString tex_path;
        if (material->GetTexture(aiTextureType_DIFFUSE, 0, &tex_path) == aiReturn_SUCCESS) {
            std::string full_path = directory + tex_path.C_Str();
            result.base_color_texture.Load(full_path);
        }

        // Load the normal map texture from the material, if any.
        if (material->GetTexture(aiTextureType_NORMALS, 0, &tex_path) == aiReturn_SUCCESS) {
            std::string full_path = directory + tex_path.C_Str();
            result.normal_map_texture.Load(full_path);
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

void Model3D::ComputeSmoothNormals(Mesh& mesh) {
    const size_t vertex_count = mesh.vertices.size();
    if (vertex_count == 0) {
        return;
    }

    // Accumulate area-weighted face normals into each vertex.
    // The cross product of two edges yields a vector whose magnitude is
    // proportional to twice the triangle area, so larger faces contribute
    // more to the smooth normal than smaller ones.
    std::vector<glm::vec3> accum(vertex_count, glm::vec3(0.0f));

    for (size_t i = 0; i + 2 < mesh.indices.size(); i += 3) {
        uint32_t i0 = mesh.indices[i];
        uint32_t i1 = mesh.indices[i + 1];
        uint32_t i2 = mesh.indices[i + 2];

        const glm::vec3& p0 = mesh.vertices[i0].position;
        const glm::vec3& p1 = mesh.vertices[i1].position;
        const glm::vec3& p2 = mesh.vertices[i2].position;

        // Area-weighted face normal (magnitude = 2 * triangle area).
        glm::vec3 face_normal = glm::cross(p1 - p0, p2 - p0);

        accum[i0] += face_normal;
        accum[i1] += face_normal;
        accum[i2] += face_normal;
    }

    for (size_t v = 0; v < vertex_count; ++v) {
        float len = glm::length(accum[v]);
        if (len > 1e-6f) {
            mesh.vertices[v].smooth_normal = accum[v] / len;
        } else {
            mesh.vertices[v].smooth_normal = mesh.vertices[v].normal;
        }
    }
}

bool Model3D::ReplaceAllTextures(const std::string& path) {
    Texture2D new_texture;
    if (!new_texture.Load(path)) {
        return false;
    }

    // Move the loaded texture into the first mesh and reload from the same
    // path for every subsequent mesh so each owns its own GPU handle.
    for (size_t i = 0; i < meshes_.size(); ++i) {
        if (i == 0) {
            meshes_[i].base_color_texture = std::move(new_texture);
        } else {
            meshes_[i].base_color_texture.Unload();
            meshes_[i].base_color_texture.Load(path);
        }
    }

    return true;
}

void Model3D::ComputeTangentBitangent(Mesh& mesh) {
    const size_t vertex_count = mesh.vertices.size();
    if (vertex_count == 0) {
        return;
    }

    // Initialize tangent and bitangent to zero for accumulation.
    std::vector<glm::vec3> tangent_accum(vertex_count, glm::vec3(0.0f));
    std::vector<glm::vec3> bitangent_accum(vertex_count, glm::vec3(0.0f));

    // For each triangle, compute tangent and bitangent vectors.
    for (size_t i = 0; i + 2 < mesh.indices.size(); i += 3) {
        uint32_t i0 = mesh.indices[i];
        uint32_t i1 = mesh.indices[i + 1];
        uint32_t i2 = mesh.indices[i + 2];

        const glm::vec3& p0 = mesh.vertices[i0].position;
        const glm::vec3& p1 = mesh.vertices[i1].position;
        const glm::vec3& p2 = mesh.vertices[i2].position;

        const glm::vec2& uv0 = mesh.vertices[i0].uv;
        const glm::vec2& uv1 = mesh.vertices[i1].uv;
        const glm::vec2& uv2 = mesh.vertices[i2].uv;

        glm::vec3 edge1 = p1 - p0;
        glm::vec3 edge2 = p2 - p0;

        glm::vec2 duv1 = uv1 - uv0;
        glm::vec2 duv2 = uv2 - uv0;

        float denom = duv1.x * duv2.y - duv2.x * duv1.y;
        if (std::abs(denom) < 1e-6f) {
            // Degenerate UV triangle, skip tangent computation for this face
            continue;
        }

        float f = 1.0f / denom;

        glm::vec3 tangent = (edge1 * duv2.y - edge2 * duv1.y) * f;
        glm::vec3 bitangent = (edge2 * duv1.x - edge1 * duv2.x) * f;

        // Accumulate for each vertex of the triangle
        tangent_accum[i0] += tangent;
        tangent_accum[i1] += tangent;
        tangent_accum[i2] += tangent;

        bitangent_accum[i0] += bitangent;
        bitangent_accum[i1] += bitangent;
        bitangent_accum[i2] += bitangent;
    }

    // Orthogonalize and normalize tangent and bitangent for each vertex.
    for (size_t v = 0; v < vertex_count; ++v) {
        const glm::vec3& normal = mesh.vertices[v].normal;
        glm::vec3 tangent = tangent_accum[v];

        // Gram-Schmidt orthogonalization
        tangent = glm::normalize(tangent - normal * glm::dot(normal, tangent));

        // Ensure handedness (bitangent direction relative to tangent and normal)
        float handedness = glm::dot(glm::cross(normal, tangent), bitangent_accum[v]) < 0.0f ? -1.0f : 1.0f;

        mesh.vertices[v].tangent = tangent;
        mesh.vertices[v].bitangent = handedness * glm::normalize(glm::cross(normal, tangent));
    }
}

bgfx::VertexLayout Model3D::GetVertexLayout() {
    bgfx::VertexLayout layout;
    layout
        .begin()
        .add(bgfx::Attrib::Position,  3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Normal,    3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
        .add(bgfx::Attrib::TexCoord1, 1, bgfx::AttribType::Float)
        .add(bgfx::Attrib::TexCoord2, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::TexCoord3, 3, bgfx::AttribType::Float)  // tangent
        .add(bgfx::Attrib::TexCoord4, 3, bgfx::AttribType::Float)  // bitangent
        .end();
    return layout;
}
