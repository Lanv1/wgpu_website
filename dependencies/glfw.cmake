include(FetchContent)

message("Fetching glfw lib ...")
FetchContent_Declare(
    glfw
    # URL https://github.com/glfw/glfw/releases/download/3.3.8/glfw-3.3.8.zip
    # URL_HASH SHA256=4d025083cc4a3dd1f91ab9b9ba4f5807193823e565a5bcf4be202669d9911ea6
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3-stable
)
message("glfw lib fetched.")

