add_library(
    nuklear_sfml
    nuklear_sfml_gl3.cpp
    #demos.cpp
    nuklear_impl.cpp
)
target_compile_options(nuklear_sfml PRIVATE -fPIE)	