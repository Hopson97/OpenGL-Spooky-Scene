#pragma once

#include <SFML/Window/Event.hpp>
#include <SFML/Window/Window.hpp>
#include <glad/glad.h>

#include <filesystem>
#include <iostream>
#include <string_view>
#include <vector>

namespace fs = std::filesystem;

class Shader
{
  public:
    Shader() = default;
    bool load_from_file(const fs::path& vertex_file_path, const fs::path& fragment_file_path);

    void bind() const;

  private:
    GLuint program_ = 0;
};