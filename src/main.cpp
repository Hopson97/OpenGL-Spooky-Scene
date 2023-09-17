#include <SFML/Graphics/Image.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Window.hpp>
#include <glad/glad.h>
#include <glm/glm.hpp>

#include "GLDebugEnable.h"
#include "Shader.h"
#include "Util.h"

#include <nuklear/nuklear_def.h>
#include <nuklear/nuklear_sfml_gl3.h>
struct Vertex
{
    glm::vec3 position{0.0f};
    glm::vec3 colour{0.0f};
    glm::vec2 texture{0.0f};
};

namespace GUI
{
    nk_context* ctx = nullptr;

    int window_flags = NK_WINDOW_BORDER | NK_WINDOW_SCALABLE | NK_WINDOW_MOVABLE |
                       NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_SCALE_LEFT | NK_WINDOW_MINIMIZABLE;

    void init(sf::Window* window)
    {
        ctx = nk_sfml_init(window);

        nk_font_atlas* atlas;
        nk_sfml_font_stash_begin(&atlas);
        nk_sfml_font_stash_end();
    }

    void begin_frame()
    {
        assert(ctx);
        nk_input_begin(ctx);
    }

    void end_frame()
    {
        assert(ctx);
        nk_input_end(ctx);
    }

    void shutdown()
    {
        assert(ctx);
        nk_sfml_shutdown();
    }

    void render()
    {
        assert(ctx);
        nk_sfml_render(NK_ANTI_ALIASING_ON, 0x80000, 0x80000);
    }

    void event(sf::Event& e)
    {
        assert(ctx);
        nk_sfml_handle_event(&e);
    }

    void show_debug_window()
    {
        assert(ctx);
        if (nk_begin(ctx, "Debug Window", nk_rect(10, 10, 300, 130), window_flags))
        {
            nk_layout_row_dynamic(ctx, 12, 1);
            nk_labelf(ctx, NK_STATIC, "Hello World");
            nk_end(ctx);
        }
    }

} // namespace GUI

int main()
{
    sf::ContextSettings context_settings;
    context_settings.depthBits = 24;
    context_settings.stencilBits = 8;
    context_settings.antialiasingLevel = 4;
    context_settings.majorVersion = 4;
    context_settings.minorVersion = 5;
    context_settings.attributeFlags = sf::ContextSettings::Core;

    sf::Window window({1280, 720}, "Matt GL", sf::Style::Default, context_settings);
    window.setVerticalSyncEnabled(true);

    window.setActive(true);

    if (!gladLoadGL())
    {
        std::cerr << "Failed to init OpenGL - Is OpenGL linked correctly?\n";
        return -1;
    }
    glViewport(0, 0, 1280, 720);
    // glClearColor(1.0, 1.0, 1.0, 1.0);

    init_opengl_debugging();

    /*
        Init GUI
    */

    GUI::init(&window);

    std::vector<Vertex> points = {{{0.5f, 0.5f, 0.0f}, {0.2f, 0.2f, 0.5f}, {0.0f, 1.0f}},
                                  {{-0.5f, 0.5f, 0.0f}, {0.2f, 0.5f, 0.5f}, {1.0f, 1.0f}},
                                  {{-0.5f, -0.5f, 0.0f}, {0.2f, 0.5f, 1.0f}, {1.0f, 0.0f}},
                                  {{0.5f, -0.5f, 0.0f}, {0.2f, 0.5f, 0.5f}, {0.0f, 0.0f}}

    };

    // std::vector<GLfloat> points = {
    //     , , , ,
    // };
    std::vector<GLuint> indices = {0, 1, 2, 2, 3, 0};

    // ----------------------------------------
    // ==== Create the OpenGL vertex array ====
    // ----------------------------------------
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ebo = 0;

    // Create the OpenGL buffer objects
    glCreateVertexArrays(1, &vao);
    glCreateBuffers(1, &vbo);
    glCreateBuffers(1, &ebo);

    // == Init the element buffer ==
    glNamedBufferStorage(ebo, indices.size() * sizeof(GLuint), indices.data(), 0x0);
    glVertexArrayElementBuffer(vao, ebo);

    // glBufferData
    // glNamedBufferStorage(vbo, points.size() * sizeof(Vertex), points.data(), 0x0);
    glNamedBufferStorage(vbo, sizeof(Vertex) * points.size(), points.data(),
                         GL_DYNAMIC_STORAGE_BIT);

    // Attach the vertex array to the vertex buffer and element buffer
    glVertexArrayVertexBuffer(vao, 0, vbo, 0, sizeof(Vertex));

    // glEnableVertexAttribArray
    glEnableVertexArrayAttrib(vao, 0);
    glEnableVertexArrayAttrib(vao, 1);
    glEnableVertexArrayAttrib(vao, 2);

    // glVertexAttribPointer
    glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, position));
    glVertexArrayAttribFormat(vao, 1, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, colour));
    glVertexArrayAttribFormat(vao, 2, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, texture));
    glVertexArrayAttribBinding(vao, 0, 0);
    glVertexArrayAttribBinding(vao, 1, 0);
    glVertexArrayAttribBinding(vao, 2, 0);

    // -----------------------------------
    // ==== Create the OpenGL Texture ====
    // -----------------------------------
    GLuint texture_handle;
    glCreateTextures(GL_TEXTURE_2D, 1, &texture_handle);

    // Load the texture from file
    sf::Image image;
    image.loadFromFile("assets/textures/person.png");
    image.flipVertically();
    image.flipHorizontally();
    auto w = image.getSize().x;
    auto h = image.getSize().y;
    auto data = image.getPixelsPtr();

    // Upload the texture to the GPU
    glTextureStorage2D(texture_handle, 1, GL_RGBA8, w, h);
    glTextureSubImage2D(texture_handle, 0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, data);

    // Set texture wrapping and min/mag filters
    glTextureParameteri(texture_handle, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(texture_handle, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(texture_handle, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTextureParameteri(texture_handle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // ----------------------
    // ==== Load shaders ====
    // ----------------------
    Shader shader;
    shader.load_from_file("assets/shaders/vertex.glsl", "assets/shaders/fragment.glsl");

    // -------------------
    // ==== Main Loop ====
    // -------------------
    while (window.isOpen())
    {
        GUI::begin_frame();
        sf::Event e;
        while (window.pollEvent(e))
        {
            GUI::event(e);
            if (e.type == sf::Event::Closed)
            {
                window.close();
            }
        }
        if (!window.isOpen())
        {
            break;
        }

        GUI::end_frame();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        GUI::show_debug_window();

        // Set the render states
        glBindTextureUnit(0, texture_handle);
        shader.bind();
        glBindVertexArray(vao);

        // Render
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);

        // Un-set render states
        glBindVertexArray(0);
        glUseProgram(0);
        glBindTextureUnit(0, 0);

        GUI::render();

        window.display();
    }

    GUI::shutdown();
}