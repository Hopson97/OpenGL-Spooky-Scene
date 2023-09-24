#include "MeshGeneration.h"

#include <numeric>


#include <SFML/Graphics/Image.hpp>

/*
Cool blue RGB:

{0.2f, 0.2f, 0.5f},
{0.2f, 0.5f, 0.5f},
{0.2f, 0.5f, 1.0f},
{0.2f, 0.5f, 0.5f},
*/

Mesh generate_quad_mesh(float w, float h)
{
    Mesh mesh;
    mesh.vertices = {{{w, h, 0.0f},  {0.0f, 1.0f}, {0, 0, 1}},
                     {{0, h, 0.0f},  {1.0f, 1.0f}, {0, 0, 1}},
                     {{0, 0, 0.0f},  {1.0f, 0.0f}, {0, 0, 1}},
                     {{w, 0, 0.0f},  {0.0f, 0.0f}, {0, 0, 1}}

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
        {{w, h, d}, {1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},  
        {{0, h, d}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
        {{0, 0, d}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},  
        {{w, 0, d}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},

        {{0, h, d}, {1.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}}, 
        {{0, h, 0}, {0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}},
        {{0, 0, 0}, {0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}}, 
        {{0, 0, d}, {1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}},

        {{0, h, 0}, {1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}}, 
        {{w, h, 0}, {0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}},
        {{w, 0, 0}, {0.0f, 1.0f}, {0.0f, 0.0f, -1.0f}}, 
        {{0, 0, 0}, {1.0f, 1.0f}, {0.0f, 0.0f, -1.0f}},

        {{w, h, 0}, {1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},  
        {{w, h, d}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
        {{w, 0, d}, {0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},  
        {{w, 0, 0}, {1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},

        {{w, h, 0}, {1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},  
        {{0, h, 0}, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        {{0, h, d}, {0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},  
        {{w, h, d}, {1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},

        {{0, 0, 0}, {1.0f, 0.0f}, {0.0f, -1.0f, 0.0f}}, 
        {{w, 0, 0}, {0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}},
        {{w, 0, d}, {0.0f, 1.0f}, {0.0f, -1.0f, 0.0f}}, 
        {{0, 0, d}, {1.0f, 1.0f}, {0.0f, -1.0f, 0.0f}},
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

Mesh generate_terrain_mesh(int size)
{
    float sizef = static_cast<float>(size);

    Mesh mesh;
    for (int z = 0; z < size; z++)
    {
        for (int x = 0; x < size; x++)
        {
            GLfloat fz = static_cast<GLfloat>(z);
            GLfloat fx = static_cast<GLfloat>(x);

            Vertex vertex;
            vertex.position.x = fx;
            vertex.position.y = 0.0f;
            vertex.position.z = fz;

            vertex.texture_coord.s = fx;
            vertex.texture_coord.t = fz;

            vertex.normal = {0, 1, 0};

            mesh.vertices.push_back(vertex);
        }
    }

    for (int z = 0; z < size - 1; z++)
    {
        for (int x = 0; x < size - 1; x++)
        {
            int topLeft = (z * size) + x;
            int topRight = topLeft + 1;
            int bottomLeft = ((z + 1) * size) + x;
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

bool Model::load_from_file(const fs::path& path)
{
    auto path_str = path.string();
    // Load the model, other options include aiProcess_GenNormals, aiProcess_SplitLargeMeshes,
    // aiProcess_OptimizeMeshes
    Assimp::Importer importer;
    auto scene = importer.ReadFile(path_str, aiProcess_Triangulate | aiProcess_FlipUVs |
                                                 aiProcess_GenNormals); //
    //|
    // aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph);

    if (!scene || !scene->mRootNode || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
    {
        std::cerr << "Could not load model " << path << "\n"
                  << importer.GetErrorString() << '\n';
        return false;
    }

    directory = path_str.substr(0, path_str.find_last_of('/'));
    process_node(scene->mRootNode, scene);

    int vertex_count = 0;
    int indices_count = 0;
    int textures_count = 0;

    for (auto& mesh : meshes)
    {
        vertex_count += mesh.vertices.size();
        indices_count += mesh.indices.size();
        textures_count += mesh.textures.size();
    }

    std::cout << "Loaded " << path << "\nMeshes: " << meshes.size()
              << "\nVertices: " << vertex_count << "\nIndices: " << indices_count
              << "\nTexutres: " << textures_count << '\n';
    return true;
}

void Model::process_node(aiNode* node, const aiScene* scene)
{
    for (unsigned i = 0; i < node->mNumMeshes; i++)
    {
        auto mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(process_mesh(mesh, scene));
    }

    for (unsigned i = 0; i < node->mNumChildren; i++)
    {
        process_node(node->mChildren[i], scene);
    }
}

GLuint load_texture(const fs::path& path)
{
    std::cout << "Loading texture " << path << '\n';
    GLuint texture;
    glCreateTextures(GL_TEXTURE_2D, 1, &texture);

    // Load the texture from file
    sf::Image image;
    image.loadFromFile(path.string());
    image.flipVertically();
    auto w = image.getSize().x;
    auto h = image.getSize().y;
    auto data = image.getPixelsPtr();

    // Set the storage
    glTextureStorage2D(texture, 8, GL_RGBA8, w, h);
    // glGenerateMipmap(GL_TEXTURE_2D);

    // Upload the texture to the GPU to cover the whole created texture
    glTextureSubImage2D(texture, 0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateTextureMipmap(texture);

    // Set texture wrapping and min/mag filters
    glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return texture;
};

std::vector<Texture> Model::load_material(aiMaterial* material, aiTextureType texture_type)
{
    std::vector<Texture> textures;

    for (unsigned i = 0; i < material->GetTextureCount(texture_type); i++)
    {
        aiString str;
        material->GetTexture(texture_type, i, &str);

        bool should_load = true;
        for (auto& cached : texture_cache)
        {
            if (cached.path == std::string(str.C_Str()))
            {
                textures.push_back(cached);
                should_load = false;
                break;
            }
        }

        if (should_load)
        {
            Texture texture;

            texture.type = [texture_type]()
            {
                switch (texture_type)
                {

                    case aiTextureType_DIFFUSE:
                        return "diffuse";
                        break;
                    case aiTextureType_SPECULAR:
                        return "specular";
                        break;
                    default:
                        return "Unknown";
                }
            }();

            texture.path = str.C_Str();
            texture.id = load_texture(directory + "/" + str.C_Str());
            textures.push_back(texture);
            texture_cache.push_back(texture);
        }
    }

    return textures;
}

Mesh Model::process_mesh(aiMesh* ai_mesh, const aiScene* scene)
{
    Mesh mesh;

    // Process the Assimp's mesh vertices
    for (unsigned i = 0; i < ai_mesh->mNumVertices; i++)
    {
        Vertex v;

        v.position.x = ai_mesh->mVertices[i].x;
        v.position.y = ai_mesh->mVertices[i].y;
        v.position.z = ai_mesh->mVertices[i].z;

        if (ai_mesh->HasNormals())
        {

            v.normal.x = ai_mesh->mNormals[i].x;
            v.normal.y = ai_mesh->mNormals[i].y;
            v.normal.z = ai_mesh->mNormals[i].z;
        }
        if (ai_mesh->mTextureCoords[0])
        {
            v.texture_coord.x = ai_mesh->mTextureCoords[0][i].x;
            v.texture_coord.y = ai_mesh->mTextureCoords[0][i].y;
        }
        else
        {
            v.texture_coord = {0.0, 0.0};
        }

        mesh.vertices.push_back(v);
    }

    // Process Indices
    for (unsigned i = 0; i < ai_mesh->mNumFaces; i++)
    {
        auto face = ai_mesh->mFaces[i];
        for (unsigned j = 0; j < face.mNumIndices; j++)
        {
            mesh.indices.push_back(face.mIndices[j]);
        }
    }

    if (ai_mesh->mMaterialIndex >= 0)
    {
        auto material = scene->mMaterials[ai_mesh->mMaterialIndex];
        auto diffuse_maps = load_material(material, aiTextureType_DIFFUSE);
        auto specular_maps = load_material(material, aiTextureType_SPECULAR);

        mesh.textures.insert(mesh.textures.end(), diffuse_maps.begin(), diffuse_maps.end());
        mesh.textures.insert(mesh.textures.end(), specular_maps.begin(), specular_maps.end());
    }

    return mesh;
}
