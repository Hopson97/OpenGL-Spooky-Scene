#pragma once

#include <filesystem>
#include <iostream>
#include <string_view>
#include <vector>

namespace fs = std::filesystem;

std::string read_file_to_string(const std::filesystem::path& file_path);