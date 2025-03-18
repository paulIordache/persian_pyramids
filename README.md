# OpenGL 3D Graphics Project 🖥️🎮

This project is a 3D graphics application using OpenGL, which includes a custom rendering pipeline with several advanced features. The project showcases various lighting techniques, shadows, and interactive camera movement for exploring a 3D model in real time. The goal is to create a visually immersive experience with custom-made shaders, including elements of Physically-Based Rendering (PBR), spotlight, point light, directional light, and shadow mapping.

## Features ✨

- **Lighting Models 💡**: 
  - **Directional Light 🌞**: Simulates light coming from a specific direction (like sunlight).
  - **Point Light 🔆**: Simulates light that radiates in all directions from a point source.
  - **Spotlight 🔦**: Focuses light in a specific direction, like a flashlight.
  
- **Shadow Mapping 🌑**:
  - **Point Light Shadows 🌑**: Shadows are generated from a point light source using shadow mapping techniques.
  
- **Physically-Based Rendering (PBR) 🌍**: 
  - Implements basic PBR techniques to simulate realistic material behavior.
  
- **Camera Movement 🎥**:
  - Uses a custom camera system that allows the user to explore the 3D scene with keyboard and mouse controls.
  
- **Model Tour System 🗺️**: 
  - A system that enables users to explore the 3D model by providing predefined tour paths and automated camera movements.
  
- **Custom Shaders 🎨**:
  - Vertex and fragment shaders are written from scratch for controlling lighting, shadows, and materials.
  - Includes shaders for PBR, shadow mapping, and model rendering.

- **Audio 🎶**:
  - Background music plays while exploring the model (Persian Mystical Music).

## Prerequisites ⚙️

Before running the project, ensure you have the following installed:

- **CMake 🛠️**: For building the project.
- **GLEW ⚡**: For OpenGL extension loading.
- **GLFW 🌐**: For creating windows and managing input.
- **OpenGL 🖥️**: Required for rendering.
- **Audio Library 🎧**: For background music (if needed).

## Build Instructions 🏗️

1. **Clone the repository**:
    ```bash
    git clone https://github.com/yourusername/yourproject.git
    cd yourproject
    ```

2. **Create a build directory**:
    ```bash
    mkdir build
    cd build
    ```

3. **Run CMake**:
    ```bash
    cmake ..
    ```

4. **Build the project**:
    ```bash
    cmake --build .
    ```

5. **Run the application**:
    After building, you can run the application with:
    ```bash
    ./lab_11.exe
    ```

## Usage 🎮

1. **Navigate with the camera 🕹️**:
    - Use **WASD** to move the camera.
    - Use the **mouse** to look around the scene.

2. **Tour System 🚶**:
    - The camera will automatically follow a predefined tour path to showcase the 3D model.

3. **Lighting 💡**:
    - The scene features a combination of directional light, point light, and spotlight, which can be seen as you explore the model.

4. **Shadows 🌑**:
    - Shadows are dynamically calculated, especially from the point light source.

5. **Audio 🎶**:
    - Background music (Persian Mystical Music) will play automatically when the scene starts.

## Custom Shaders 🎨

The project includes several custom shaders to handle lighting, materials, and shadows:

- **PBR Shader 🏙️**: Implements basic Physically-Based Rendering.
- **Shadow Mapping Shader 🌑**: Used for point light shadows.
- **Lighting Shader 💡**: Manages different types of lighting (directional, point, spotlight).
- **Skybox Shader 🌌**: Renders the skybox as the background.

