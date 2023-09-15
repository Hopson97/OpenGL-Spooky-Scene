#include <SFML/Window/Event.hpp>
#include <SFML/Window/Window.hpp>
#include <glad/glad.h>
#include <glm/glm.hpp>

#include "GLDebugEnable.h"
#include "Shader.h"
#include "Util.h"

struct Vertex
{
    glm::vec3 position;
    glm::vec3 colour;
};

int main()
{
    sf::ContextSettings context_shaders;
    context_shaders.depthBits = 24;
    context_shaders.stencilBits = 8;
    context_shaders.antialiasingLevel = 4;
    context_shaders.majorVersion = 4;
    context_shaders.minorVersion = 5;
    context_shaders.attributeFlags = sf::ContextSettings::Core;

    sf::Window window({1280, 720}, "Matt GL", sf::Style::Default, context_shaders);
    window.setVerticalSyncEnabled(true);

    window.setActive(true);

    if (!gladLoadGL())
    {
        std::cerr << "Failed to init OpenGL - Is OpenGL linked correctly?\n";
        return -1;
    }
    glViewport(0, 0, 1280, 720);
    glClearColor(1.0, 1.0, 1.0, 1.0);

    init_opengl_debugging();

    std::vector<Vertex> points = {{{0.5f, 0.5f, 0.0f}, {0.5f, 0.5f, 0.5f}},
                                  {{-0.5f, 0.5f, 0.0f}, {0.5f, 0.5f, 0.5f}},
                                  {{-0.5f, -0.5f, 0.0f}, {0.5f, 0.5f, 0.5f}},
                                  {{0.5f, -0.5f, 0.0f}, {0.5f, 0.5f, 0.5f}}

    };

    // std::vector<GLfloat> points = {
    //     , , , ,
    // };
    std::vector<GLuint> indices = {0, 1, 2, 2, 3, 0};

    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ebo = 0;

    GLuint buffer = 0;
    glCreateVertexArrays(1, &vao);
    glCreateBuffers(1, &vbo);
    glCreateBuffers(1, &ebo);

    /*
            Buffer the data and attatch to the VAO
    */
    // = glBufferData(BUFFER_TYPE, sizeof(GLfloat) * points.size(),  points.data(),
    // = GL_STATIC_DRAW);
    glNamedBufferStorage(vbo, sizeof(Vertex) * points.size(), points.data(),
                         GL_DYNAMIC_STORAGE_BIT);
    glVertexArrayVertexBuffer(vao, 0, vbo, 0, sizeof(Vertex));

    // = glEnableVertexAttribArray(0);
    glEnableVertexArrayAttrib(vao, 0);
    glEnableVertexArrayAttrib(vao, 1);

    // = glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
    glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, position));
    glVertexArrayAttribFormat(vao, 1, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, colour));

    glVertexArrayAttribBinding(vao, 0, 0);
    glVertexArrayAttribBinding(vao, 1, 0);

    /*
        Buffer the index buffer and attatch to the VAO
    */
    glNamedBufferStorage(ebo, sizeof(GLuint) * indices.size(), indices.data(),
                         GL_DYNAMIC_STORAGE_BIT);
    glVertexArrayElementBuffer(vao, ebo);

    /*
        Buffer the index buffer and attatch to the VAO
    */
    Shader shader;
    shader.load_from_file("assets/shaders/vertex.glsl", "assets/shaders/fragment.glsl");

    while (window.isOpen())
    {
        sf::Event e;
        while (window.pollEvent(e))
        {
            if (e.type == sf::Event::Closed)
            {
                window.close();
            }
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.bind();
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);

        window.display();
    }
}