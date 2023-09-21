#include <SFML/Graphics/Image.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Window.hpp>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <nuklear_sfml/nuklear_def.h>
#include <nuklear_sfml/nuklear_sfml_gl3.h>

#include <imgui_sfml/imgui-SFML.h>
#include <imgui_sfml/imgui_impl_opengl3.h>

#include "GLDebugEnable.h"
#include "MeshGeneration.h"
#include "Shader.h"
#include "Util.h"

namespace
{
    struct Transform
    {
        glm::vec3 position{0.0f};
        glm::vec3 rotation{0.0f};
    };

    struct VertexArray
    {
        GLuint vao = 0;
        GLuint vbo = 0;
        GLuint ebo = 0;
    };

    struct Settings
    {
        bool wireframe = false;
        float material_shine = 32.0f;

        glm::vec3 light_direction{0.0f, -1.0f, 0.0f};
        glm::vec3 light_colour{0.8f, 0.8f, 0.8f};
        float ambient_intensitiy = 1.0f;
        float diffuse_intensity = 1.0f;

        glm::vec3 point_light_colour{0.8f, 0.8f, 0.8f};
        float point_ambient_intensitiy = 1.0f;
        float point_diffuse_intensity = 1.0f;
        float point_attenuation_constant = 1.0f;
        float point_attenuation_linear = 0.045f;
        float point_attenuation_exponant = 0.0075f;

        /*
        glm::vec3 light_diffuse{0.6f, 0.6f, 0.6f};
        glm::vec3 light_specular{1.0f, 1.0f, 1.0f};
        float light_att_linear = 0.045f;
        float light_att_exponant = 0.0075f;

        float spotlight_cutoff = 12.5f;
        */
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

    glm::vec3 get_keyboard_input(const Transform& transform, bool flying)
    {

        auto x_rot = glm::radians(transform.rotation.x);
        auto y_rot = glm::radians(transform.rotation.y);
        auto y_rot90 = glm::radians(transform.rotation.y + 90);

        // Keyboard Input
        glm::vec3 move{0.0f};
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
        {
            move +=
                glm::vec3{glm::cos(y_rot) * glm::cos(x_rot), flying ? glm::sin(x_rot) : 0.0f,
                          glm::cos(x_rot) * glm::sin(y_rot)};
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
        {
            move -=
                glm::vec3{glm::cos(y_rot) * glm::cos(x_rot), flying ? glm::sin(x_rot) : 0.0f,
                          glm::cos(x_rot) * glm::sin(y_rot)};
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
        {
            move += glm::vec3{-glm::cos(y_rot90), 0, -glm::sin(y_rot90)};
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
        {
            move -= glm::vec3{-glm::cos(y_rot90), 0, -glm::sin(y_rot90)

            };
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl))
        {
            move *= 10.0f;
        }

        return move;
    }

    void get_mouse_move_input(Transform& transform, const sf::Window& window)
    {
        auto& r = transform.rotation;
        static auto last_mouse = sf::Mouse::getPosition(window);
        auto change = sf::Mouse::getPosition(window) - last_mouse;
        r.x -= static_cast<float>(change.y * 0.35);
        r.y += static_cast<float>(change.x * 0.35);
        sf::Mouse::setPosition({(int)window.getSize().x / 2, (int)window.getSize().y / 2},
                               window);
        last_mouse = sf::Mouse::getPosition(window);

        r.x = glm::clamp(r.x, -89.9f, 89.9f);
        if (r.y >= 360.0f)
        {
            r.y = 0.0f;
        }
        if (r.y < 0.0f)
        {
            r.y = 359.9f;
        }
    }
} // namespace

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

        ImGui::SFML::Init(*window, cast_vector<float>(window->getSize()));
        ImGui_ImplOpenGL3_Init();
    }

    void begin_frame()
    {
        assert(ctx);
        nk_input_begin(ctx);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();
    }

    void shutdown()
    {
        assert(ctx);
        nk_sfml_shutdown();
    }

    void render()
    {
        assert(ctx);
        nk_input_end(ctx);
        nk_sfml_render(NK_ANTI_ALIASING_ON, 0x80000, 0x80000);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void event(sf::Event& e)
    {
        assert(ctx);
        nk_sfml_handle_event(&e);
    }

    void debug_window(const Transform& transform, Settings& settings)
    {
        assert(ctx);
        auto r = transform.rotation;
        auto p = transform.position;
        if (nk_begin(ctx, "Debug Window", nk_rect(10, 10, 300, 130), window_flags))
        {
            nk_layout_row_dynamic(ctx, 12, 1);
            nk_labelf(ctx, NK_STATIC, "Position: (%f, %f, %f)", p.x, p.y, p.z);
            nk_labelf(ctx, NK_STATIC, "Rotation: (%f, %f, %f)", r.x, r.y, r.z);
        }

        nk_end(ctx);
        auto& s = settings;
        if (ImGui::Begin("Debug Window"))
        {
            ImGui::Text("Position: (%f, %f, %f)", p.x, p.y, p.z);
            ImGui::Text("Rotation: (%f, %f, %f)", r.x, r.y, r.z);

            ImGui::Separator();
            ImGui::Text("Directional Light");
            if (ImGui::SliderFloat3("Direction", &settings.light_direction[0], -1.0f, 1.0f,
                                    "%.2f"))
            {
                settings.light_direction = glm::normalize(settings.light_direction);
            }
            ImGui::SliderFloat3("Colour", &s.light_colour[0], 0.0f, 1.0f, "%.2f");
            ImGui::SliderFloat("Ambient", &s.ambient_intensitiy, 0.0f, 1.0f);
            ImGui::SliderFloat("Duiffuse", &s.diffuse_intensity, 0.0f, 1.0f);

            ImGui::Separator();
            ImGui::Text("Point Light");
            ImGui::SliderFloat3("Colourr", &s.point_light_colour[0], 0.0f, 1.0f, "%.2f");
            ImGui::SliderFloat("Ambientt", &s.point_ambient_intensitiy, 0.0f, 1.0f);
            ImGui::SliderFloat("Duiffusee", &s.point_diffuse_intensity, 0.0f, 1.0f);

            ImGui::SliderFloat("Constantt", &s.point_attenuation_constant, 0.0f, 1.0f);
            ImGui::SliderFloat("exponant", &s.point_attenuation_exponant, 0.000007f, 0.03f);
            ImGui::SliderFloat("Linear", &s.point_attenuation_linear, 0.0014f, 0.14f);


            ImGui::SliderFloat("Material Shine", &settings.material_shine, 1.0f, 64.0f);
            /*

            ImGui::SliderFloat("Light Attenuation Linear", &settings.light_att_linear, 0.14f,
                               0.0014f);
            ImGui::SliderFloat("Light Attenuation exponant", &settings.light_att_exponant,
                               0.000007f, 0.03f);


            ImGui::SliderFloat3("Light Diffuse", &settings.light_diffuse[0], 0.0f, 1.0f,
                                "%.2f");
            ImGui::SliderFloat3("Light Specular", &settings.light_specular[0], 0.0f, 1.0f,
                                "%.2f");

            ImGui::SliderFloat("Spotlight Cutoff", &settings.spotlight_cutoff, 1.0f, 90.0f);
            */
        }
        ImGui::End();
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

    sf::Window window({1600, 900}, "Matt GL", sf::Style::Default, context_settings);
    window.setVerticalSyncEnabled(true);
    bool mouse_locked = false;

    window.setActive(true);

    if (!gladLoadGL())
    {
        std::cerr << "Failed to init OpenGL - Is OpenGL linked correctly?\n";
        return -1;
    }
    glViewport(0, 0, 1600, 900);
    init_opengl_debugging();
    GUI::init(&window);

    // ---------------------------
    // ==== Create the Meshes ====
    // ---------------------------
    Mesh terrain_mesh = generate_terrain_mesh(256, 512);
    Mesh light_mesh = generate_cube_mesh({0.2f, 1.2f, 0.2f});
    Mesh box_mesh = generate_cube_mesh({3.0f, 3.0f, 3.0f});

    // ----------------------------------------
    // ==== Create the OpenGL vertex array ====
    // ----------------------------------------
    auto buffer_mesh = [](Mesh& mesh)
    {
        std::cout << "Creating vertex arrays\n";
        VertexArray vertex_array;

        // Create the OpenGL buffer objects
        glCreateVertexArrays(1, &vertex_array.vao);
        glCreateBuffers(1, &vertex_array.vbo);
        glCreateBuffers(1, &vertex_array.ebo);
        auto vao = vertex_array.vao;

        // Element buffer
        glNamedBufferStorage(vertex_array.ebo, mesh.indices.size() * sizeof(GLuint),
                             mesh.indices.data(), 0x0);
        glVertexArrayElementBuffer(vao, vertex_array.ebo);

        // glBufferData
        // glNamedBufferStorage(vbo, points.size() * sizeof(Vertex), points.data(), 0x0);
        glNamedBufferStorage(vertex_array.vbo, sizeof(Vertex) * mesh.vertices.size(),
                             mesh.vertices.data(), GL_DYNAMIC_STORAGE_BIT);

        // Attach the vertex array to the vertex buffer and element buffer
        glVertexArrayVertexBuffer(vao, 0, vertex_array.vbo, 0, sizeof(Vertex));

        // glEnableVertexAttribArray
        glEnableVertexArrayAttrib(vao, 0);
        glEnableVertexArrayAttrib(vao, 1);
        glEnableVertexArrayAttrib(vao, 2);
        glEnableVertexArrayAttrib(vao, 3);

        // glVertexAttribPointer
        glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, position));
        glVertexArrayAttribFormat(vao, 1, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, colour));
        glVertexArrayAttribFormat(vao, 2, 2, GL_FLOAT, GL_FALSE,
                                  offsetof(Vertex, texture_coord));
        glVertexArrayAttribFormat(vao, 3, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, normal));
        glVertexArrayAttribBinding(vao, 0, 0);
        glVertexArrayAttribBinding(vao, 1, 0);
        glVertexArrayAttribBinding(vao, 2, 0);
        glVertexArrayAttribBinding(vao, 3, 0);

        return vertex_array;
    };

    auto terrain_vertex_array = buffer_mesh(terrain_mesh);
    auto light_vertex_array = buffer_mesh(light_mesh);
    auto box_vertex_array = buffer_mesh(box_mesh);

    // ------------------------------------
    // ==== Create the OpenGL Textures ====
    // ------------------------------------
    auto load_texture = [](const fs::path& path)
    {
        std::cout << "Loading texture " << path << '\n';
        GLuint texture;
        glCreateTextures(GL_TEXTURE_2D, 1, &texture);

        // Load the texture from file
        sf::Image image;
        image.loadFromFile(path.string());
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

    GLuint person_texture = load_texture("assets/textures/person.png");
    GLuint grass_texture = load_texture("assets/textures/grass_03.png");
    GLuint grass_specular_texture = load_texture("assets/textures/grass_specular.png");
    GLuint crate_texture = load_texture("assets/textures/crate.png");
    GLuint specular_texture = load_texture("assets/textures/crate_specular.png");

    // ---------------------------------------
    // ==== Create the OpenGL Framebuffer ====
    // ---------------------------------------
    std::cout << "Creating framebuffer\n";
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
    if (!scene_shader.load_from_file("assets/shaders/SceneVertex.glsl",
                                     "assets/shaders/SceneFragment.glsl"))
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
    Transform terrain_transform;
    Transform light_transform;
    std::vector<Transform> box_transforms;

    for (int i = 0; i < 25; i++)
    {
        float x = static_cast<float>(rand() % 100);
        float y = static_cast<float>(rand() % 360);
        float z = static_cast<float>(rand() % 100);

        box_transforms.push_back({{x, -5.0f, z}, {0.0f, y, 0}});
    }

    camera_transform.rotation.y = 90.0f;
    camera_transform.position = {20.0f, 1.0f, 20.0f};
    light_transform.position = {20.0f, 3.0f, 20.0f};

    terrain_transform.position.y -= 5.0f;

    glm::mat4 camera_projection =
        glm::perspective(glm::radians(75.0f), 1600.0f / 900.0f, 1.0f, 100.0f);
    glm::vec3 up = {0, 1, 0};

    Settings settings;

    // -------------------
    // ==== Main Loop ====
    // -------------------
    TimeStep<60> time_step;
    sf::Clock game_time;
    sf::Clock delta_clock;
    while (window.isOpen())
    {
        auto game_time_now = game_time.getElapsedTime();
        GUI::begin_frame();
        sf::Event e;
        while (window.pollEvent(e))
        {
            ImGui::SFML::ProcessEvent(window, e);
            GUI::event(e);
            if (e.type == sf::Event::Closed)
                window.close();
            else if (e.type == sf::Event::KeyReleased)
                if (e.key.code == sf::Keyboard::Escape)
                    window.close();
                else if (e.key.code == sf::Keyboard::L)
                    mouse_locked = !mouse_locked;
        }
        if (!window.isOpen())
        {
            break;
        }
        // ImGui::SFML::Update()

        // ---------------
        // ==== Input ====
        // ---------------
        auto SPEED = 5.0f;
        auto translate = get_keyboard_input(camera_transform, true) * SPEED;

        if (!mouse_locked)
        {
            window.setMouseCursorVisible(false);
            get_mouse_move_input(camera_transform, window);
        }
        else
        {
            window.setMouseCursorVisible(true);
        }

        // ----------------------------------
        // ==== Update w/ Fixed timestep ====
        // ----------------------------------
        time_step.update(
            [&](auto dt)
            {
                camera_transform.position += translate * dt.asSeconds();

                light_transform.position.x +=
                    glm::sin(game_time_now.asSeconds() * 0.55f) * dt.asSeconds() * 3.0f;
                light_transform.position.z +=
                    glm::cos(game_time_now.asSeconds() * 0.55f) * dt.asSeconds() * 3.0f;
            });

        // -------------------------------
        // ==== Transform Calculation ====
        // -------------------------------
        glm::mat4 view_matrix{1.0f};
        auto x_rot = glm::radians(camera_transform.rotation.x);
        auto y_rot = glm::radians(camera_transform.rotation.y);
        glm::vec3 front = {
            glm::cos(y_rot) * glm::cos(x_rot),
            glm::sin(x_rot),
            glm::sin(y_rot) * glm::cos(x_rot),
        };
        glm::vec3 centre = camera_transform.position + glm::normalize(front);

        view_matrix = glm::lookAt(camera_transform.position, centre, up);

        // Calculate the terrain postion and rotation
        glm::mat4 terrain_mat{1.0f};

        terrain_mat = glm::translate(terrain_mat, terrain_transform.position);

        // Calculate the light postion and rotation
        glm::mat4 light_mat{1.0f};

        light_mat = glm::translate(light_mat, light_transform.position);

        light_mat =
            glm::rotate(light_mat, glm::radians(light_transform.rotation.x), {1, 0, 0});
        light_mat =
            glm::rotate(light_mat, glm::radians(light_transform.rotation.y), {0, 1, 0});
        light_mat =
            glm::rotate(light_mat, glm::radians(light_transform.rotation.z), {0, 0, 1});

        // Calulate the boxes
        std::vector<glm::mat4> box_mats;
        for (auto& box_transform : box_transforms)
        {
            glm::mat4 box_mat{1.0f};

            box_mat = glm::translate(box_mat, box_transform.position);

            box_mat = glm::rotate(box_mat, glm::radians(box_transform.rotation.x), {1, 0, 0});
            box_mat = glm::rotate(box_mat, glm::radians(box_transform.rotation.y), {0, 1, 0});
            box_mat = glm::rotate(box_mat, glm::radians(box_transform.rotation.z), {0, 0, 1});

            box_mats.push_back(box_mat);
        }

        // -----------------------
        // ==== Render to FBO ====
        // -----------------------
        // Set the framebuffer as the render target and clear
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        // Set the render states
        scene_shader.bind();
        scene_shader.set_uniform("projection_matrix", camera_projection);
        scene_shader.set_uniform("view_matrix", view_matrix);

        scene_shader.set_uniform("eye_position", camera_transform.position);

        scene_shader.set_uniform("material.diffuse", 0);
        scene_shader.set_uniform("material.specular", 1);
        // scene_shader.set_uniform("material.shininess", settings.material_shine);

        // // scene_shader.set_uniform("point_light.position", light_transform.position);
        //  scene_shader.set_uniform("point_light.ambient", settings.light_colour);

        /*
        scene_shader.set_uniform("point_light.diffuse", settings.light_diffuse);
        scene_shader.set_uniform("point_light.specular", settings.light_specular);
        scene_shader.set_uniform("point_light.linear", settings.light_att_linear);
        scene_shader.set_uniform("point_light.exponant", settings.light_att_exponant);
        */

        float sun_mult = 0.8f;
        //(std::sin(game_time_now.asSeconds() / 2.0f) + 1);
        scene_shader.set_uniform("sun.direction", settings.light_direction);
        scene_shader.set_uniform("sun.base.colour", settings.light_colour * sun_mult);
        scene_shader.set_uniform("sun.base.ambient_intensity", settings.ambient_intensitiy);
        scene_shader.set_uniform("sun.base.diffuse_intensity", settings.diffuse_intensity);

        scene_shader.set_uniform("point_light.base.colour",
                                 settings.point_light_colour);
        scene_shader.set_uniform("point_light.base.ambient_intensity",
                                 settings.point_ambient_intensitiy);
        scene_shader.set_uniform("point_light.base.diffuse_intensity",
                                 settings.point_diffuse_intensity);

        scene_shader.set_uniform("point_light.position", light_transform.position);
        scene_shader.set_uniform("point_light.att.constant",
                                 settings.point_attenuation_constant);
        scene_shader.set_uniform("point_light.att.linear",
                                 settings.point_attenuation_linear);
        scene_shader.set_uniform("point_light.att.exponant",
                                 settings.point_attenuation_exponant);

        // scene_shader.set_uniform("sun_light.diffuse", settings.light_diffuse * sun_mult);
        // scene_shader.set_uniform("sun_light.specular", settings.light_specular * sun_mult);
        /*
        scene_shader.set_uniform("spotlight.position", light_transform.position);
        scene_shader.set_uniform("spotlight.direction", {0,-1,0});
        scene_shader.set_uniform("spotlight.cutoff",
        glm::cos(glm::radians(settings.spotlight_cutoff)));
        //scene_shader.set_uniform("spotlight.outer_cutoff", glm::cos(glm::radians(20.5f)));
        scene_shader.set_uniform("spotlight.ambient", settings.light_colour);
        scene_shader.set_uniform("spotlight.diffuse", settings.light_diffuse);
        scene_shader.set_uniform("spotlight.specular", settings.light_specular);
        */
        scene_shader.set_uniform("is_light", false);

        // Set the terrain trasform and render
        // glBindTextureUnit(0, grass_texture);
        // glBindTextureUnit(1, grass_specular_texture);
        glBindTextureUnit(0, crate_texture);
        glBindTextureUnit(1, specular_texture);
        scene_shader.set_uniform("model_matrix", terrain_mat);
        glBindVertexArray(terrain_vertex_array.vao);
        glDrawElements(GL_TRIANGLES, terrain_mesh.indices.size(), GL_UNSIGNED_INT, nullptr);

        // Set the box transforms and render
        glBindTextureUnit(0, crate_texture);
        glBindTextureUnit(1, specular_texture);
        glBindVertexArray(box_vertex_array.vao);
        for (auto& box_matrix : box_mats)
        {
            scene_shader.set_uniform("model_matrix", box_matrix);
            glDrawElements(GL_TRIANGLES, box_mesh.indices.size(), GL_UNSIGNED_INT, nullptr);
        }

        // Set the light trasform and render
        scene_shader.set_uniform("is_light", true);
        scene_shader.set_uniform("model_matrix", light_mat);
        glBindVertexArray(light_vertex_array.vao);
        glDrawElements(GL_TRIANGLES, light_mesh.indices.size(), GL_UNSIGNED_INT, nullptr);

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
        GUI::debug_window(camera_transform, settings);

        GUI::render();
        window.display();
    }

    GUI::shutdown();

    // Cleanup OpenGL
    glDeleteBuffers(1, &terrain_vertex_array.vbo);
    glDeleteBuffers(1, &terrain_vertex_array.ebo);
    glDeleteVertexArrays(1, &terrain_vertex_array.vao);

    glDeleteTextures(1, &person_texture);

    glDeleteFramebuffers(1, &fbo);
}