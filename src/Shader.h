#pragma once

#include <SFML/Window/Event.hpp>
#include <SFML/Window/Window.hpp>
#include <glad/glad.h>
#include <glm/glm.hpp>

#include <filesystem>
#include <iostream>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace fs = std::filesystem;

class Shader
{
  public:
    Shader() = default;
    Shader(Shader&& other) noexcept = delete;
    Shader(const Shader& other) = delete;
    Shader& operator=(Shader&& other) noexcept = delete;
    Shader& operator=(const Shader& other) = delete;
    ~Shader();

    bool load_from_file(const fs::path& vertex_file_path, const fs::path& fragment_file_path);

    void bind() const;

    void set_uniform(const std::string& name, const glm::mat4& matrix);

  private:
    GLuint get_uniform_location(const std::string& name);

  private:
    std::unordered_map<std::string, GLuint> uniform_locations_;
    GLuint program_ = 0;
};