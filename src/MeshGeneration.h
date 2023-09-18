#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

struct Vertex
{
    glm::vec3 position{0.0f};
    glm::vec3 colour{0.0f};
    glm::vec2 texture_coord{0.0f};
    glm::vec3 normal{0.0f};
};

struct Mesh
{
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
};

Mesh generate_terrain_mesh(int size, int edgeVertices);