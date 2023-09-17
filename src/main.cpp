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
    std::cout << "Creating vertex arrays" << std::endl;
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
    std::cout << "Creating texture" << std::endl;

    GLuint person_texture;
    glCreateTextures(GL_TEXTURE_2D, 1, &person_texture);

    // Load the texture from file
    sf::Image image;
    image.loadFromFile("assets/textures/person.png");
    image.flipVertically();
    image.flipHorizontally();
    auto w = image.getSize().x;
    auto h = image.getSize().y;
    auto data = image.getPixelsPtr();

    // Set the storage
    glTextureStorage2D(person_texture, 1, GL_RGBA8, w, h);

    // Upload the texture to the GPU to cover the whole created texture
    glTextureSubImage2D(person_texture, 0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, data);

    // Set texture wrapping and min/mag filters
    glTextureParameteri(person_texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(person_texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(person_texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTextureParameteri(person_texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // ---------------------------------------
    // ==== Create the OpenGL Framebuffer ====
    // ---------------------------------------
    std::cout << "Creating framebuffer" << std::endl;
    auto fbo_x = window.getSize().x;
    auto fbo_y = window.getSize().y;
    GLuint fbo;
    glCreateFramebuffers(1, &fbo);

    // Attach the texture to the framebuffer
    GLuint fbo_texture;
    glCreateTextures(GL_TEXTURE_2D, 1, &fbo_texture);
    glTextureStorage2D(fbo_texture, 1, GL_RGB8, fbo_x, fbo_y);

    glTextureParameteri(fbo_texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(fbo_texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glNamedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, fbo_texture, 0);

    // Attatch a render buffer to the frame buffer
    GLuint rbo;
    glCreateRenderbuffers(1, &rbo);
    glNamedRenderbufferStorage(rbo, GL_DEPTH24_STENCIL8, fbo_x, fbo_y);
    glNamedFramebufferRenderbuffer(fbo, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (auto status =
            glCheckNamedFramebufferStatus(fbo, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "Framebuffer incomplete. Status: " << status << '\n';
        return -1;
    }

    // --------------------------------------------------
    // ==== Create empty VBO for rendering to window ====
    // --------------------------------------------------
    GLuint fbo_vbo;
    glCreateVertexArrays(1, &fbo_vbo);

    // ----------------------
    // ==== Load shaders ====
    // ----------------------
    Shader scene_shader;
    if (!scene_shader.load_from_file("assets/shaders/vertex.glsl",
                                     "assets/shaders/fragment.glsl"))
    {
        return -1;
    }

    Shader fbo_shader;
    if (!fbo_shader.load_from_file("assets/shaders/ScreenVertex.glsl",
                                   "assets/shaders/ScreenFragment.glsl"))
    {
        return -1;
    }

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
        glBindTextureUnit(0, person_texture);
        scene_shader.bind();
        glBindVertexArray(vao);

        // Set the framebuffer as the render target and clear
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Render
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);

        // Un-set render states
        glBindVertexArray(0);
        glUseProgram(0);
        glBindTextureUnit(0, 0);

        // Prepare final render pass, so unbind the FBO and clear screen
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Bind the FBOs texture which will texture the screen quad
        glBindTextureUnit(0, fbo_texture);
        fbo_shader.bind();

        glBindVertexArray(fbo_vbo);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        GUI::render();

        window.display();
    }

    GUI::shutdown();

    // Cleanup OpenGL
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    glDeleteVertexArrays(1, &vao);

    glDeleteTextures(1, &person_texture);

    glDeleteFramebuffers(1, &fbo);
}