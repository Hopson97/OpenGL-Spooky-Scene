#include "MeshGeneration.h"

Mesh generate_quad_mesh(float size)
{
    Mesh mesh;
    mesh.vertices = {{{size, size, 0.0f}, {0.2f, 0.2f, 0.5f}, {0.0f, 1.0f}, {0, 0, 1}},
                     {{-size, size, 0.0f}, {0.2f, 0.5f, 0.5f}, {1.0f, 1.0f}, {0, 0, 1}},
                     {{-size, -size, 0.0f}, {0.2f, 0.5f, 1.0f}, {1.0f, 0.0f}, {0, 0, 1}},
                     {{size, -size, 0.0f}, {0.2f, 0.5f, 0.5f}, {0.0f, 0.0f}, {0, 0, 1}}

    };

    mesh.indices = {0, 1, 2, 2, 3, 0};

    return mesh;
}

Mesh generate_cube_mesh(const glm::vec3& dimensions)
{
    Mesh mesh;

    float w = dimensions.x;
    float h = dimensions.y;
    float d = dimensions.z;

    // clang-format off
    mesh.vertices = {
        {{w, h, d}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},  
        {{0, h, d}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
        {{0, 0, d}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},  
        {{w, 0, d}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},

        {{0, h, d}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}}, 
        {{0, h, 0}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}},
        {{0, 0, 0}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}}, 
        {{0, 0, d}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}},

        {{0, h, 0}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}}, 
        {{w, h, 0}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}},
        {{w, 0, 0}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, -1.0f}}, 
        {{0, 0, 0}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, -1.0f}},

        {{w, h, 0}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},  
        {{w, h, d}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
        {{w, 0, d}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},  
        {{w, 0, 0}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},

        {{w, h, 0}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},  
        {{0, h, 0}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        {{0, h, d}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},  
        {{w, h, d}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},

        {{0, 0, 0}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, -1.0f, 0.0f}}, 
        {{w, 0, 0}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}},
        {{w, 0, d}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, -1.0f, 0.0f}}, 
        {{0, 0, d}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, -1.0f, 0.0f}},
    };
    // clang-format on

    int index = 0;
    for (int i = 0; i < 6; i++)
    {
        mesh.indices.push_back(index);
        mesh.indices.push_back(index + 1);
        mesh.indices.push_back(index + 2);
        mesh.indices.push_back(index + 2);
        mesh.indices.push_back(index + 3);
        mesh.indices.push_back(index);
        index += 4;
    }

    return mesh;
}

Mesh generate_terrain_mesh(int size, int edgeVertices)
{
    float fEdgeVertexCount = static_cast<float>(edgeVertices);

    Mesh mesh;
    for (int z = 0; z < edgeVertices; z++)
    {
        for (int x = 0; x < edgeVertices; x++)
        {
            GLfloat fz = static_cast<GLfloat>(z);
            GLfloat fx = static_cast<GLfloat>(x);

            int hx = x + 1;
            int hz = z + 1;

            Vertex vertex;
            vertex.position.x = fx / fEdgeVertexCount * size;
            vertex.position.y = 0.0f;
            vertex.position.z = fz / fEdgeVertexCount * size;

            vertex.texture_coord.s = (fx / fEdgeVertexCount) * edgeVertices / 4.0f;
            vertex.texture_coord.t = (fz / fEdgeVertexCount) * edgeVertices / 4.0f;

            vertex.colour = {1.0f, 1.0f, 1.0f};

            vertex.normal = {0, 1, 0};

            mesh.vertices.push_back(vertex);
        }
    }

    for (int z = 0; z < edgeVertices - 1; z++)
    {
        for (int x = 0; x < edgeVertices - 1; x++)
        {
            int topLeft = (z * edgeVertices) + x;
            int topRight = topLeft + 1;
            int bottomLeft = ((z + 1) * edgeVertices) + x;
            int bottomRight = bottomLeft + 1;

            mesh.indices.push_back(topLeft);
            mesh.indices.push_back(bottomLeft);
            mesh.indices.push_back(topRight);
            mesh.indices.push_back(topRight);
            mesh.indices.push_back(bottomLeft);
            mesh.indices.push_back(bottomRight);
        }
    }

    return mesh;
}