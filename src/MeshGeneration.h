#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Util.h"

struct Vertex
{
    glm::vec3 position{0.0f};
    glm::vec3 colour{0.0f};
    glm::vec2 texture_coord{0.0f};
    glm::vec3 normal{0.0f};
};

struct VertexArray
{
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ebo = 0;
};

struct Texture
{
    GLuint id = 0;
    std::string type;
    std::string path;
};

struct Mesh
{
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    std::vector<Texture> textures;

    VertexArray vertex_array;
    
};

struct Model
{
    bool load_from_file(const fs::path& path);

    void process_node(aiNode* node, const aiScene* scene);
    Mesh process_mesh(aiMesh* mesh, const aiScene* scene);
    std::vector<Texture> load_material(aiMaterial* material, aiTextureType texture_type,
                                       std::string name);

    std::vector<Texture> texture_cache;

    std::vector<Mesh> meshes;
    std::string directory;
};

[[nodiscard]] Mesh generate_quad_mesh(float w, float h);
[[nodiscard]] Mesh generate_cube_mesh(const glm::vec3& size);
[[nodiscard]] Mesh generate_terrain_mesh(int size, int edgeVertices);