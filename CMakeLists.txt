
cmake_minimum_required(VERSION 3.0...3.25)
project(
    WebGPUWebsite # name of the project, which will also be the name of the visual studio solution if you use it
    VERSION 0.1.0 # any version number
    LANGUAGES CXX C # programming languages used by the project
)

add_executable(App
    Context.h
    Context.cpp
    main.cpp
)

add_subdirectory(dependencies)

add_subdirectory(webgpu)
add_subdirectory(glfw3webgpu)


if (MSVC)
    target_compile_options(App PRIVATE /W4)
else()
    target_compile_options(App PRIVATE -Wall -Wextra -pedantic)
endif()

if(XCODE)
    set_target_properties(App PROPERTIES
        XCODE_GENERATE_SCHEME ON
        XCODE_SCHEME_ENABLE_GPU_FRAME_CAPTURE_MODE "Metal")
endif()

target_link_libraries(App PRIVATE glfw webgpu glfw3webgpu)

set_target_properties(App PROPERTIES
    CXX_STANDARD 17
    COMPILE_WARNING_AS_ERROR ON
)

# target_copy_webgpu_binaries(App)
