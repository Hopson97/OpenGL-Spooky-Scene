#include <SFML/Graphics/Image.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Window.hpp>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "GLDebugEnable.h"
#include "Shader.h"
#include "Util.h"

#include <nuklear_sfml/nuklear_def.h>
#include <nuklear_sfml/nuklear_sfml_gl3.h>

struct Vertex
{
    glm::vec3 position{0.0f};
    glm::vec3 colour{0.0f};
    glm::vec2 texture{0.0f};
};

struct Transform
{
    glm::vec3 position{0.0f};
    glm::vec3 rotation{0.0f};
};

template <int Ticks>
class TimeStep
{
  public:
    template <typename F>
    void update(F f)
    {
        sf::Time time = timer_.getElapsedTime();
        sf::Time elapsed = time - last_time_;
        last_time_ = time;
        lag_ += elapsed;
        while (lag_ >= timePerUpdate_)
        {
            lag_ -= timePerUpdate_;
            f(dt_.restart());
        }
    }

  private:
    const sf::Time timePerUpdate_ = sf::seconds(1.f / Ticks);
    sf::Clock timer_;
    sf::Clock dt_;
    sf::Time last_time_ = sf::Time::Zero;
    sf::Time lag_ = sf::Time::Zero;
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

    void debug_window(const Transform& transform)
    {
        assert(ctx);
        auto r = transform.rotation;
        auto p = transform.position;
        if (nk_begin(ctx, "Debug Window", nk_rect(10, 10, 300, 130), window_flags))
        {
            nk_layout_row_dynamic(ctx, 12, 1);
            nk_labelf(ctx, NK_STATIC, "Position: (%f, %f, %f)", p.x, p.y, p.z);
            nk_labelf(ctx, NK_STATIC, "Rotation: (%f, %f, %f)", r.x, r.y, r.z);
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

    // -----------------------------------
    // ==== Entity Transform Creation ====
    // -----------------------------------
    Transform camera_transform;
    Transform quad_transform;

    camera_transform.rotation.y = 90.0f;

    quad_transform.position.z = 3.0f;

    glm::mat4 camera_projection =
        glm::perspective(glm::radians(75.0f), 1280.0f / 720.0f, 1.0f, 100.0f);
    glm::vec3 up = {0, 1, 0};

    // -------------------
    // ==== Main Loop ====
    // -------------------
    TimeStep<60> time_step;
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
        // ---------------
        // ==== Input ====
        // ---------------
        auto x_rot = glm::radians(camera_transform.rotation.x);
        auto y_rot = glm::radians(camera_transform.rotation.y);
        auto y_rot90 = glm::radians(camera_transform.rotation.y + 90);
        auto SPEED = 5.0f;

        // Keyboard Input
        glm::vec3 move{0.0f};
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
        {
            move += glm::vec3{
                glm::cos(y_rot) * glm::cos(x_rot),
                0, // glm::sin(x_rot),
                glm::cos(x_rot) * glm::sin(y_rot),
            };
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
        {
            move -= glm::vec3{
                glm::cos(y_rot) * glm::cos(x_rot),
                0, // glm::sin(x_rot),
                glm::cos(x_rot) * glm::sin(y_rot),
            };
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
        {
            move += glm::vec3{
                -glm::cos(y_rot90),
                0,
                -glm::sin(y_rot90),
            };
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
        {
            move -= glm::vec3{
                -glm::cos(y_rot90),
                0,
                -glm::sin(y_rot90),
            };
        }

        move *= SPEED;

        {
            auto& r = camera_transform.rotation;
            static auto last_mouse = sf::Mouse::getPosition();
            auto change = sf::Mouse::getPosition() - last_mouse;
            r.x -= static_cast<float>(change.y * 0.35);
            r.y += static_cast<float>(change.x * 0.35);
           // sf::Mouse::setPosition({(int)window.getSize().x / 2, (int)window.getSize().y / 2},
            //                       window);
            last_mouse = sf::Mouse::getPosition();

            r.x = glm::clamp(r.x, -89.9f, 89.9f);
            if (r.y > 360.0f)
            {
                r.y = 0.0f;
            }
            else if (r.y < 0.0f)
            {
                r.y = 360.0f;

            }
        }


        // Update...
        time_step.update(
            [&](auto dt) { camera_transform.position += move * dt.asSeconds();
            });

        // -------------------------------
        // ==== Transform Calculation ====
        // -------------------------------
        glm::mat4 view_matrix{1.0f};
        {

            glm::vec3 front = {
                glm::cos(y_rot) * glm::cos(x_rot),
                glm::sin(x_rot),
                glm::sin(y_rot) * glm::cos(x_rot),
            };
            glm::vec3 centre = camera_transform.position + glm::normalize(front);

            view_matrix = glm::lookAt(camera_transform.position, centre, up);
        }

        glm::mat4 matrix{1.0f};

        matrix = glm::translate(matrix, quad_transform.position);

        matrix = glm::rotate(matrix, glm::radians(quad_transform.rotation.x), {1, 0, 0});
        matrix = glm::rotate(matrix, glm::radians(quad_transform.rotation.y), {0, 1, 0});
        matrix = glm::rotate(matrix, glm::radians(quad_transform.rotation.z), {0, 0, 1});

        // -----------------------
        // ==== Render to FBO ====
        // -----------------------
        // Set the framebuffer as the render target and clear
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Set the render states
        glBindTextureUnit(0, person_texture);
        glBindVertexArray(vao);
        scene_shader.bind();
        scene_shader.set_uniform("projection_matrix", camera_projection);
        scene_shader.set_uniform("view_matrix", view_matrix);
        scene_shader.set_uniform("model_matrix", matrix);

        // Render
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);

        // --------------------------
        // ==== Render to window ====
        // --------------------------
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Bind the FBOs texture which will texture the screen quad
        glBindTextureUnit(0, fbo_texture);
        glBindVertexArray(fbo_vbo);
        fbo_shader.bind();

        // Render
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // --------------------------
        // ==== End Frame ====
        // --------------------------
        GUI::debug_window(camera_transform);

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