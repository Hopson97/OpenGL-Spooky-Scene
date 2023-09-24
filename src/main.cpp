#include <array>
#include <numbers>

#include <SFML/Graphics/Image.hpp>
#include <SFML/Window/Event.hpp>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "GLDebugEnable.h"
#include "GUI.h"
#include "Lights.h"
#include "MeshGeneration.h"
#include "Shader.h"
#include "Util.h"

#include <imgui.h>

#include <SFML/Audio/Music.hpp>
#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>

namespace
{
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

int main()
{
    sf::ContextSettings context_settings;
    context_settings.depthBits = 24;
    context_settings.stencilBits = 8;
    context_settings.antialiasingLevel = 4;
    context_settings.majorVersion = 4;
    context_settings.minorVersion = 5;
    context_settings.attributeFlags = sf::ContextSettings::Core;

    sf::Window window({1600, 900}, "SpookyGL", sf::Style::Default, context_settings);
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
    Mesh billboard_mesh = generate_quad_mesh(1.0f, 2.0f);
    Mesh terrain_mesh = generate_terrain_mesh(128, 128);
    Mesh light_mesh = generate_cube_mesh({0.2f, 0.2f, 0.2f});
    Mesh box_mesh = generate_cube_mesh({2.0f, 2.0f, 2.0f});

    Model backpack;
    backpack.load_from_file("assets/models/backpack/backpack.obj");

    // ----------------------------------------
    // ==== Create the OpenGL vertex array ====
    // ----------------------------------------
    auto buffer_mesh = [](Mesh& mesh)
    {
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

    auto billboard_vertex_array = buffer_mesh(billboard_mesh);
    auto terrain_vertex_array = buffer_mesh(terrain_mesh);
    auto light_vertex_array = buffer_mesh(light_mesh);
    auto box_vertex_array = buffer_mesh(box_mesh);

    for (auto& mesh : backpack.meshes)
    {
        mesh.vertex_array = buffer_mesh(mesh);
    }

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

    GLuint person_texture = load_texture("assets/textures/person.png");
    GLuint person_specular = load_texture("assets/textures/person_specular.png");

    GLuint grass_texture = load_texture("assets/textures/grass_03.png");
    GLuint grass_specular = load_texture("assets/textures/grass_specular.png");

    GLuint crate_texture = load_texture("assets/textures/crate.png");
    GLuint crate_specular_texture = load_texture("assets/textures/crate_specular.png");

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
        float x = static_cast<float>(rand() % 120) + 3;
        float z = static_cast<float>(rand() % 120) + 3;
        float r = static_cast<float>(rand() % 360);

        box_transforms.push_back({{x, 0.0f, z}, {0.0f, r, 0}});
    }

    std::vector<Transform> people_transforms;
    for (int i = 0; i < 50; i++)
    {
        float x = static_cast<float>(rand() % 120) + 3;
        float z = static_cast<float>(rand() % 120) + 3;

        people_transforms.push_back({{x, 0.0f, z}, {0.0f, 0.0, 0}});
    }

    camera_transform.position = {80.0f, 1.0f, 35.0f};
    camera_transform.rotation = {0.0f, 201.0f, 0.0f};
    light_transform.position = {20.0f, 5.0f, 20.0f};

    glm::mat4 camera_projection =
        glm::perspective(glm::radians(75.0f), 1600.0f / 900.0f, 1.0f, 256.0f);
    glm::vec3 up = {0, 1, 0};

    // ----------------------------
    // ==== Load sound effects ====
    // ----------------------------
    // Load walking sounds
    sf::SoundBuffer walk0;
    walk0.loadFromFile("assets/sounds/sfx_step_grass_l.ogg");

    sf::SoundBuffer walk1;
    walk1.loadFromFile("assets/sounds/sfx_step_grass_r.ogg");

    std::size_t sound_idx = 0;
    std::array<sf::Sound, 2> walk_sounds;
    walk_sounds[0].setBuffer(walk0);
    walk_sounds[1].setBuffer(walk1);

    auto create_looping_bg =
        [](sf::Music& background_sfx, const std::string path, int volume, int offset)
    {
        background_sfx.openFromFile(path);
        background_sfx.setLoop(true);
        background_sfx.setVolume(volume);
        background_sfx.setPlayingOffset(sf::seconds(offset));
        background_sfx.play();
    };

    // Load ambient night sounds
    sf::Music ambient_night1;
    sf::Music ambient_night2;
    sf::Music spookysphere;
    create_looping_bg(ambient_night1, "assets/sounds/crickets.ogg", 50, 0);
    create_looping_bg(ambient_night2, "assets/sounds/crickets.ogg", 50, 5);
    create_looping_bg(spookysphere, "assets/sounds/Atmosphere_003(Loop).wav", 10, 0);

    // -------------------
    // ==== Main Loop ====
    // -------------------
    Settings settings;

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
            GUI::event(window, e);
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

        // ------------------------
        // ==== Sound handling ====
        // ------------------------
        // Walking sound effects
        if ((std::abs(translate.x + translate.y + translate.z) > 0) &&
            camera_transform.position.y > 0.5 && camera_transform.position.y < 1.5)
        {
            if (walk_sounds[sound_idx].getStatus() != sf::Sound::Status::Playing)
            {
                sound_idx++;
                if (sound_idx >= walk_sounds.size())
                {
                    sound_idx = 0;
                }

                walk_sounds[sound_idx].play();
            }
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

                //   settings.spot_light.cutoff -= 0.01;
            });

        // -------------------------------
        // ==== Transform Calculations ====
        // -------------------------------
        // View/ Camera matrix
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

        // Model matrices
        auto create_model_matrix = [](const Transform& transform)
        {
            glm::mat4 mat{1.0f};
            mat = glm::translate(mat, transform.position);
            mat = glm::rotate(mat, glm::radians(transform.rotation.x), {1, 0, 0});
            mat = glm::rotate(mat, glm::radians(transform.rotation.y), {0, 1, 0});
            mat = glm::rotate(mat, glm::radians(transform.rotation.z), {0, 0, 1});

            return mat;
        };

        auto terrain_mat = create_model_matrix(terrain_transform);
        auto light_mat = create_model_matrix(light_transform);

        std::vector<glm::mat4> box_mats;
        for (auto& box_transform : box_transforms)
        {
            box_mats.push_back(create_model_matrix(box_transform));
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

        // Set the shader states
        //......................
        scene_shader.bind();
        scene_shader.set_uniform("projection_matrix", camera_projection);
        scene_shader.set_uniform("view_matrix", view_matrix);

        scene_shader.set_uniform("eye_position", camera_transform.position);

        scene_shader.set_uniform("material.diffuse0", 0);
        scene_shader.set_uniform("material.specular0", 1);
        scene_shader.set_uniform("material.shininess", settings.material_shine);
        // clang-format off

        auto upload_base_light =
            [](Shader& shader, const LightBase& light, const std::string& uniform)
        {
            shader.set_uniform(uniform + ".base.colour",               light.colour);
            shader.set_uniform(uniform + ".base.ambient_intensity",    light.ambient_intensity);
            shader.set_uniform(uniform + ".base.diffuse_intensity",    light.diffuse_intensity);
            shader.set_uniform(uniform + ".base.specular_intensity",   light.specular_intensity);
        };
        auto upload_attenuation =
            [](Shader& shader, const Attenuation& attenuation, const std::string& uniform)
        {
            shader.set_uniform(uniform + ".att.constant",   attenuation.constant);
            shader.set_uniform(uniform + ".att.linear",     attenuation.linear);
            shader.set_uniform(uniform + ".att.exponant",   attenuation.exponant);
        };

        // Set the directional light shader uniforms
        scene_shader.set_uniform("dir_light.direction", settings.dir_light.direction);
        upload_base_light(scene_shader,                 settings.dir_light, "dir_light");

        // Set the point light shader uniforms
        scene_shader.set_uniform("point_light.position", light_transform.position);
        upload_base_light(scene_shader,                     settings.point_light, "point_light");
        upload_attenuation(scene_shader,                    settings.point_light.att, "point_light");

        // Set the spot light shader uniforms
        scene_shader.set_uniform("spot_light.cutoff",       glm::cos(glm::radians(settings.spot_light.cutoff)));
        scene_shader.set_uniform("spot_light.position",     camera_transform.position);
        scene_shader.set_uniform("spot_light.direction",    front);
        upload_base_light(scene_shader,                     settings.spot_light, "spot_light");
        upload_attenuation(scene_shader,                    settings.spot_light.att, "spot_light");

        // clang-format on

        scene_shader.set_uniform("is_light", false);

        // Set the terrain trasform and render
        if (settings.grass)
        {
            glBindTextureUnit(0, grass_texture);
            glBindTextureUnit(1, grass_specular);
        }
        else
        {
            glBindTextureUnit(0, crate_texture);
            glBindTextureUnit(1, crate_specular_texture);
        }

        scene_shader.set_uniform("model_matrix", terrain_mat);
        glBindVertexArray(terrain_vertex_array.vao);
        glDrawElements(GL_TRIANGLES, terrain_mesh.indices.size(), GL_UNSIGNED_INT, nullptr);

        // Set the box transforms and render
        glBindTextureUnit(0, crate_texture);
        glBindTextureUnit(1, crate_specular_texture);
        glBindVertexArray(box_vertex_array.vao);
        for (auto& box_matrix : box_mats)
        {
            scene_shader.set_uniform("model_matrix", box_matrix);
            glDrawElements(GL_TRIANGLES, box_mesh.indices.size(), GL_UNSIGNED_INT, nullptr);
        }

        // Draws a mesh by loop the textures to bind, and then rendering
        auto draw_model = [](const Mesh& mesh, Shader& shader)
        {
            GLuint diffuse_id = 0;
            GLuint specular_id = 0;
            for (int i = 0; i < mesh.textures.size(); i++)
            {
                std::string number;
                std::string name = mesh.textures[i].type;
                if (name == "diffuse")
                    number = std::to_string(diffuse_id++);
                else if (name == "specular")
                    number = std::to_string(specular_id++);

                auto uni = "material." + name + number;
                shader.set_uniform(uni, i);

                glBindTextureUnit(i, mesh.textures[i].id);
            }
            // draw mesh
            glBindVertexArray(mesh.vertex_array.vao);
            glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        };

        // Draw a model loaded from assimp
        glm::mat4 mesh_matrix{1.0f};
        mesh_matrix = glm::translate(mesh_matrix, {30.0f, 5.0f, 30.0f});
        // mesh_matrix = glm::scale(mesh_matrix, {0.02f, 0.02f, 0.02f});
        // mesh_matrix = glm::scale(mesh_matrix, {10.0f, 10.0f, 10.0f});
        scene_shader.set_uniform("model_matrix", mesh_matrix);
        for (auto& mesh : backpack.meshes)
        {
            draw_model(mesh, scene_shader);
        }

        // Draw billboards
        glBindTextureUnit(0, person_texture);
        glBindTextureUnit(1, person_specular);
        glBindVertexArray(billboard_vertex_array.vao);
        for (auto& transform : people_transforms)
        {

            // Draw billboard
            auto pi = static_cast<float>(std::numbers::pi);
            auto xd = transform.position.x - camera_transform.position.x;
            auto yd = transform.position.z - camera_transform.position.z;

            auto r = std::atan2(xd, yd) + pi;

            glm::mat4 billboard_mat{1.0f};
            billboard_mat = glm::translate(billboard_mat, transform.position);
            billboard_mat = glm::rotate(billboard_mat, r, {0, 1, 0});

            scene_shader.set_uniform("model_matrix", billboard_mat);
            glDrawElements(GL_TRIANGLES, billboard_mesh.indices.size(), GL_UNSIGNED_INT,
                           nullptr);
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
        // ImGui::ShowDemoWindow();
        GUI::debug_window(camera_transform.position, camera_transform.rotation, settings);

        GUI::render();
        window.display();
    }

    // --------------------------
    // ==== Graceful Cleanup ====
    // --------------------------
    GUI::shutdown();

    // Delete all vertex arrays
    auto cleanup_vertex_array = [](VertexArray& vertex_array)
    {
        glDeleteBuffers(1, &vertex_array.vbo);
        glDeleteBuffers(1, &vertex_array.ebo);
        glDeleteVertexArrays(1, &vertex_array.vao);
    };
    cleanup_vertex_array(billboard_vertex_array);
    cleanup_vertex_array(terrain_vertex_array);
    cleanup_vertex_array(light_vertex_array);
    cleanup_vertex_array(box_vertex_array);

    for (auto& mesh : backpack.meshes)
    {
        cleanup_vertex_array(mesh.vertex_array);
    }

    // Delete all textures
    glDeleteTextures(1, &person_texture);
    glDeleteTextures(1, &person_specular);

    glDeleteTextures(1, &crate_texture);
    glDeleteTextures(1, &crate_specular_texture);

    glDeleteTextures(1, &grass_texture);
    glDeleteTextures(1, &grass_specular);

    // Delete all framebuffers...
    glDeleteFramebuffers(1, &fbo);
    glDeleteFramebuffers(1, &fbo_texture);
}