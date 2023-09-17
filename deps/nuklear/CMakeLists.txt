add_library(
    nuklear
    nuklear_sfml_gl3.cpp
    #demos.cpp
    nuklear_impl.cpp
)
target_compile_options(nuklear PRIVATE -fPIE)	