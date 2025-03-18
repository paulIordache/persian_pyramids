cmake_minimum_required(VERSION 3.25)
project(lab_11)

set(CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++ -static-libgcc")

# Add executable
add_executable(lab_11 main.cpp Shader.cpp Camera.cpp Mesh.cpp Model3D.cpp stb_image.cpp tiny_obj_loader.cpp SkyBox.cpp
        "SurveillanceCamera .h")

# Find required packages
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(OpenCV CONFIG REQUIRED)
find_package(SDL2_mixer CONFIG REQUIRED)
find_package(SDL2 REQUIRED)
find_package(SndFile CONFIG REQUIRED)
find_package(OpenAL CONFIG REQUIRED)
find_package(SFML COMPONENTS system window graphics CONFIG REQUIRED)


# Add ImGui library
add_library(imgui
        third_party/imgui/imgui.cpp
        third_party/imgui/imgui_draw.cpp
        third_party/imgui/imgui_widgets.cpp
        third_party/imgui/imgui_tables.cpp
        third_party/imgui/imgui_demo.cpp
        third_party/imgui/backends/imgui_impl_glfw.cpp
        third_party/imgui/backends/imgui_impl_opengl3.cpp
)

# Include directories for ImGui
target_include_directories(imgui PUBLIC
        third_party/imgui
        third_party/imgui/backends
)

# Link libraries
target_link_libraries(lab_11 PRIVATE
        opencv_ml
        opencv_dnn
        opencv_core
        opencv_flann
        GLEW::GLEW
        imgui
        glfw
        glm::glm
        SDL2_mixer::SDL2_mixer
        $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
        $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
        SndFile::sndfile
        OpenAL::OpenAL
        sfml-system sfml-network sfml-graphics sfml-window
        sfml-main
)