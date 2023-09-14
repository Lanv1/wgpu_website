cmake_minimum_required(VERSION 3.1...3.25)
project(
	WebGpuWebsite
	VERSION 0.1.0
	LANGUAGES CXX C
)

# We add an option to enable different settings when developping the app than
# when distributing it.
option(DEV_MODE "Set up development helper settings" OFF)


add_subdirectory(webgpu)
add_subdirectory(glfw3webgpu)

add_executable(App
	main.cpp
	Context.h
	Context.cpp
	implementations.cpp
)


target_include_directories(App PRIVATE .)

target_link_libraries(App PRIVATE glfw webgpu glfw3webgpu)

set_target_properties(App PROPERTIES
	CXX_STANDARD 17
	VS_DEBUGGER_ENVIRONMENT "DAWN_DEBUG_BREAK_ON_ERROR=1"
)
target_copy_webgpu_binaries(App)

if (MSVC)
	# Ignore a warning that GLM requires to bypass
	# Disable warning C4201: nonstandard extension used: nameless struct/union
	target_compile_options(App PUBLIC /wd4201)
	# Disable warning C4305: truncation from 'int' to 'bool' in 'if' condition
	target_compile_options(App PUBLIC /wd4305)

	# Ignore a warning that stb_image requires to bypass
	# Disable warning C4244: conversion from 'int' to 'short', possible loss of data
	target_compile_options(App PUBLIC /wd4244)
endif (MSVC)

# At the end of the CMakeLists.txt
if (EMSCRIPTEN)
	# Add Emscripten-specific link options
	target_link_options(App PRIVATE
		-sUSE_GLFW=3 # Use Emscripten-provided GLFW
		-sUSE_WEBGPU # Handle WebGPU symbols
		-sASYNCIFY # Required by WebGPU-C++
	)

	# Generate a full web page rather than a simple WebAssembly module
	set_target_properties(App PROPERTIES SUFFIX ".html")
endif()
