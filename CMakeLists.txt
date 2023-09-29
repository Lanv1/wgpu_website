cmake_minimum_required(VERSION 3.1...3.25)
project(
	WebGpuWebsite
	VERSION 0.1.0
	LANGUAGES CXX C
)

# We add an option to enable different settings when developping the app than
# when distributing it.
option(DEV_MODE "Set up development helper settings" ON)
set(SHELL_FILE shell.html)

add_subdirectory(webgpu)
add_subdirectory(glfw3webgpu)
add_subdirectory(dependencies)

add_executable(App
	main.cpp
	Context.h
	Context.cpp
	Application.h
	Application.cpp
	GpuProcess.h
	GpuProcess.cpp
	PipelineRenderTriangle.h
	PipelineRenderTriangle.cpp
	PipelineRenderSdf.h
	PipelineRenderSdf.cpp
	implementations.cpp
)

if(DEV_MODE)
	# In dev mode, we load resources from the source tree, so that when we
	# dynamically edit resources (like shaders), these are correctly
	# versionned.
	target_compile_definitions(App PRIVATE
		RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/resources"
	)
else()
	# In release mode, we just load resources relatively to wherever the
	# executable is launched from, so that the binary is portable
	target_compile_definitions(App PRIVATE
		RESOURCE_DIR="./resources"
	)
endif()



target_include_directories(App PRIVATE .)

target_link_libraries(App PRIVATE glfw webgpu glfw3webgpu glm)

set_target_properties(App PROPERTIES
	CXX_STANDARD 17
	VS_DEBUGGER_ENVIRONMENT "DAWN_DEBUG_BREAK_ON_ERROR=1"
)
target_copy_webgpu_binaries(App)


# Add Emscripten-specific link options
target_link_options(App PRIVATE
	-sUSE_GLFW=3 # Use Emscripten-provided GLFW
	-sUSE_WEBGPU # Handle WebGPU symbols
	-sASYNCIFY # Required by WebGPU-C++
	--preload-file "${CMAKE_CURRENT_SOURCE_DIR}/resources"
	-sALLOW_MEMORY_GROWTH
	--shell-file "${CMAKE_CURRENT_SOURCE_DIR}/${SHELL_FILE}"
)

# Re-link app whenever shell or shaders are modified.
file(GLOB SHADERS "${CMAKE_CURRENT_SOURCE_DIR}/resources/*")
set_property(
    TARGET App
    PROPERTY LINK_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/${SHELL_FILE}" "${SHADERS}"
)

# Generate a full web page rather than a simple WebAssembly module
set_target_properties(App PROPERTIES SUFFIX ".html")
