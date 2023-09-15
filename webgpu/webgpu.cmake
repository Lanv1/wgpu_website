include(FetchContent)

FetchContent_Declare(
	webgpu-backend-emscripten
	GIT_REPOSITORY https://github.com/eliemichel/WebGPU-distribution
	GIT_TAG        emscripten
)
FetchContent_MakeAvailable(webgpu-backend-emscripten)


