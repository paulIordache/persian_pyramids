//
//  main.cpp
//  OpenGL Advances Lighting
//
//  Created by CGIS on 28/11/16.
//  Copyright   2016 CGIS. All rights reserved.
//

#if defined (_APPLE_)
#define GLFW_INCLUDE_GLCOREARB
    #define GL_SILENCE_DEPRECATION
#else
#define GLEW_STATIC
#include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>

#include <GLFW/glfw3.h>
#include "glm/glm.hpp"//core glm functionality
#include "glm/gtc/matrix_transform.hpp"//glm extension for generating common transformation matrices
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_inverse.hpp"

#include "Shader.hpp"
#include "ShaderG.hpp"
#include "Model3D.hpp"
#include "Camera.hpp"
#include "SkyBox.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "SurveillanceCamera .h"
#include <opencv2/opencv.hpp>
//#include <SDL2/SDL.h>
//#include <SDL2/SDL_mixer.h>
#include <SFML/Audio.hpp>
#include <AL/al.h>
#include <AL/alc.h>
#include <sndfile.h>


#include <cmath>
#include <iostream>

int glWindowWidth = 2600;
int glWindowHeight = 1600;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;

    const unsigned int SHADOW_WIDTH = 4096;
const unsigned int SHADOW_HEIGHT = 4096;

glm::mat4 model;
GLuint modelLoc;
glm::mat4 view;
GLuint viewLoc;
glm::mat4 projection;
GLuint projectionLoc;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;
glm::mat4 lightRotation;

glm::vec3 lightDir;
GLuint lightDirLoc;
glm::vec3 lightColor;
GLuint lightColorLoc;



std::vector<glm::vec3> lpos = {
        glm::vec3(29.19f, 1.01f, 12.18f),
        glm::vec3(-25.35f, 65.46f, 9.06f),
        glm::vec3(47.86, 53.41, -36.80),
        glm::vec3(67.27, 46.08, -5.58),
        glm::vec3(35.37, 72.59, -34.13),
        glm::vec3(49.70, 51.78, -17.84),
        glm::vec3(-96.08, 40.84, -5.27),
        glm::vec3(62.44, 42.23, 25.34),
        glm::vec3(42.40, 22.56, -27.11),
        glm::vec3(72.39, 14.73, 16.30),
        glm::vec3(136.38, 41.41, -8.19),
        glm::vec3(140.04, 42.88, 40.00),
        glm::vec3(46.20, 0.22, 9.68),
        glm::vec3(-121.16, 38.83, 16.45),
        glm::vec3(-122.31, 39.98, -23.11),
        glm::vec3(62.22, 2.41, 7.68),
        glm::vec3(-56.65, 20.36, 25.16),
        glm::vec3(57.21, 20.56, -1.44),
        glm::vec3(-58.86, 20.46, -26.59),
        glm::vec3(-57.09, 19.48, -1.79)

};

const int numSpotLights = 3; // Define the number of spotlights
glm::vec3 spotLightPositions[] = {
        glm::vec3(-18.68, 3.43, -3.24),
        glm::vec3(-18.68, 3.43, -3.24)
};
glm::vec3 spotLightDirections[] = {
        glm::vec3(2.f, -1.f, 1.f),
        glm::vec3(.5f, -1.f, -2.f)
};
float spotLightCutOffs[] = {
        glm::cos(glm::radians(15.0f)),
        glm::cos(glm::radians(20.0f))
};
float spotLightOuterCutOffs[] = {
        glm::cos(glm::radians(20.0f)),
        glm::cos(glm::radians(25.0f))
};

glm::vec3 lposEye;

glm::vec3 spos;
glm::vec3 sposEye;

float constant = 1.0f;   // Strong baseline intensity
float linear = 0.09f;    // Gradual falloff
float quadratic = 0.032f; // Very gradual falloff for far-reaching light


float spotLightCutOff = glm::cos(glm::radians(30.0f));    // Inner cone, wider than before
float spotLightOuterCutOff = glm::cos(glm::radians(45.0f)); // Outer cone, softer edge

gps::Camera myCamera(
        glm::vec3(0.0f, 2.0f, 5.5f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f));

gps::Camera surveillanceCamera(
        glm::vec3(0.0f, 2.0f, 5.5f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f));
float cameraSpeed = 0.01f;

bool pressedKeys[1024];
float angleY = 0.0f;
GLfloat lightAngle;

gps::Model3D nanosuit;
gps::Model3D ground;
gps::Model3D lightCube;
gps::Model3D screenQuad;

gps::Shader myCustomShader;
gps::Shader lightShader;
gps::Shader screenQuadShader;
gps::Shader depthMapShader;
gps::Shader pointShader;
gps::Shader alternateCameraShader;

GLuint shadowMapFBO;

GLuint depthMapTexture;

gps::SkyBox mySkyBox;
gps::Shader skyboxShader;

std::vector<const GLchar*> faces;

bool showDepthMap;
bool showPointMap;
glm::vec3 shelfPosition = glm::vec3(30.33f, 0.17f, 12.92f); // Initial position


struct Keyframe {
    glm::vec3 position; // Camera position
    glm::vec3 target;   // Point the camera looks at
    float timestamp;    // Time in seconds
};

// Add provided keyframes and interpolated values for smoother transitions
std::vector<Keyframe> createKeyframes() {
    return {
            {{30.11f, 0.30f, 12.66f}, {28.71f, 0.34f, 12.64f}, 0.0f},
            {{29.95f, 0.10f, 12.07f}, {28.55f, 0.23f, 12.73f}, 5.0f},   // Adjusted 5-second gap
            {{29.72f, 0.38f, 12.00f}, {28.20f, 0.14f, 13.14f}, 10.0f},  // Adjusted 5-second gap
            {{29.01f, 0.23f, 12.20f}, {28.10f, 0.27f, 12.39f}, 16.0f},
            {{28.54f, 0.39f, 13.07f}, {28.40f, 0.13f, 12.18f}, 26.0f},
            {{28.80f, 0.43f, 12.70f}, {29.26f, 0.26f, 13.29f}, 30.0f},
            {{28.80f, 0.43f, 12.70f}, {30.16f, 0.26f, 12.25f}, 35.0f},   // Adjusted 5-second gap
            {{29.13f, 0.19f, 12.07f}, {29.11f, 0.15f, 11.77f}, 40.0f},   // Adjusted 5-second gap
            {{29.41f, 0.22f, 12.39f}, {30.40f, 0.08f, 11.97f}, 45.0f},   // Adjusted 5-second gap
            {{30.06f, 0.24f, 12.85f}, {31.17f, 0.27f, 12.92f}, 50.0f},   // Adjusted 5-second gap
            {{30.65f, 0.15f, 12.98f}, {31.73f, 0.25f, 12.95f}, 55.0f},   // Adjusted 5-second gap
            {{31.23f, 0.80f, 13.49f}, {55.97f, 15.55f, 2.26f}, 60.0f},   // Adjusted 5-second gap
            {{31.23f, 0.80f, 13.49f}, {51.33f, 0.80f, 11.55f}, 65.0f},   // Adjusted 5-second gap
            {{31.70f, 0.95f, 13.28f}, {26.65f, 1.53f, 23.95f}, 70.0f},   // Adjusted 5-second gap
            {{30.96f, 0.95f, 15.06f}, {6.07f, 5.56f, 21.17f}, 75.0f},    // Adjusted 5-second gap
            {{17.29f, 5.50f, 12.75f}, {11.87f, 1.87f, 30.17f}, 80.0f},    // Adjusted 5-second gap
            {{13.80f, 3.98f, 11.71f}, {-8.65f, 3.12f, 12.32f}, 85.0f},    // Adjusted 5-second gap
            {{7.91f, 3.80f, 7.89f}, {15.20f, -0.13f, 0.64f}, 90.0f},     // Adjusted 5-second gap
            {{2.14f, 3.89f, -1.00f}, {11.15f, 0.27f, -9.86f}, 95.0f},    // Adjusted 5-second gap
            {{0.15f, 4.21f, -21.42f}, {11.15f, 0.27f, -9.86f}, 100.0f}, // Adjusted 5-second gap
            {{11.90f, 3.73f, -29.37f}, {-12.87f, 21.84f, -14.84f}, 105.0f}, // Adjusted 5-second gap
            {{25.27f, 10.78f, -30.34f}, {39.90f, 5.00f, -12.80f}, 110.0f}, // Adjusted 5-second gap
            {{42.50f, 10.72f, -19.58f}, {66.62f, 14.72f, -18.21f}, 120.0f}, // Adjusted 5-second gap
            {{62.66f, 10.23f, -27.94f}, {77.89f, 12.78f, -21.46f}, 130.0f},
            {{72.13f, 5.04f, -18.84f}, {65.33f, 0.90f, -10.58f}, 140.0f}, // Adjusted 10-second gap
            {{62.03f, 1.01f, -11.21f}, {62.61f, 0.36f, -1.48f}, 150.0f}, // Adjusted 10-second gap
            {{55.28f, 4.92f, -3.67f}, {62.00f, 1.82f, 3.32f}, 160.0f},   // Adjusted 10-second gap
            {{55.91f, 2.22f, 6.57f}, {60.83f, 1.09f, 7.33f}, 170.0f},    // Adjusted 10-second gap
            {{48.51f, 7.17f, 5.13f}, {60.75f, 5.32f, 4.38f}, 180.0f},    // Adjusted 10-second gap
            {{36.23f, 11.31f, 19.28f}, {45.52f, 22.66f, -2.24f}, 190.0f}, // Adjusted 10-second gap
            {{36.23f, 11.31f, 19.28f}, {10.19f, 26.56f, 14.28f}, 200.0f}, // Adjusted 10-second gap
            {{38.27f, 0.59f, 18.84f}, {59.69f, 28.66f, 4.04f}, 210.0f},  // Adjusted 10-second gap
            {{31.16f, 0.35f, 13.16f}, {29.20f, 0.13f, 12.32f}, 220.0f},  // Adjusted 10-second gap
            {{28.67f, 0.23f, 12.37f}, {28.98f, 0.09f, 11.68f}, 230.0f},  // Adjusted 10-second gap
            {{28.27f, 0.27f, 12.05f}, {28.03f, 0.18f, 11.87f}, 240.0f},  // Adjusted 10-second gap
            {{28.23f, 0.20f, 11.97f}, {27.91f, 0.18f, 12.31f}, 250.0f},  // Adjusted 10-second gap
            {{28.03f, 0.23f, 11.86f}, {29.35f, 0.20f, 12.68f}, 260.0f}   // Adjusted 10-second gap
    };
}

void rotateCameraToTarget(const glm::vec3& target) {
    // Calculate the direction from the camera to the target
    glm::vec3 direction = glm::normalize(target - myCamera.getPosition());

    // Calculate yaw and pitch based on the direction vector
    float yaw = glm::atan(direction.z, direction.x); // Yaw is rotation around the Y-axis (from front vector)
    float pitch = glm::asin(direction.y); // Pitch is rotation around the X-axis

    // Update camera's yaw and pitch to face the target
    myCamera.rotate(pitch, glm::degrees(yaw)); // Rotate the camera towards the target
}

void moveAndRotateCamera(const std::vector<Keyframe>& keyframes, float& currentTime, float deltaTime) {
    static size_t keyframeIndex = 0;

    if (keyframeIndex >= keyframes.size() - 1) return;

    const Keyframe& currentKeyframe = keyframes[keyframeIndex];
    const Keyframe& nextKeyframe = keyframes[keyframeIndex + 1];

    // Calculate the time interval between keyframes
    float duration = nextKeyframe.timestamp - currentKeyframe.timestamp;

    // Increment the current time by deltaTime
    currentTime += deltaTime;

    // Calculate the interpolation factor based on current time and timestamps
    float t = glm::clamp((currentTime - currentKeyframe.timestamp) / duration, 0.0f, 1.0f);

    // Interpolate position and target
    glm::vec3 interpolatedPosition = glm::mix(currentKeyframe.position, nextKeyframe.position, t);
    glm::vec3 interpolatedTarget = glm::mix(currentKeyframe.target, nextKeyframe.target, t);

    // Update the camera's position and orientation
    myCamera.setPosition(interpolatedPosition);
    rotateCameraToTarget(interpolatedTarget);

    // Advance to the next keyframe if the interpolation is complete
    if (t >= 1.0f) {
        keyframeIndex++;
        currentTime = currentKeyframe.timestamp + duration; // Snap time to the next keyframe
    }
}

void initImGui(GLFWwindow* window) {
    std::cout << "Initializing ImGui..." << std::endl;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    if (!ImGui::GetCurrentContext()) {
        std::cerr << "Failed to create ImGui context!" << std::endl;
    }

    if (!ImGui_ImplGlfw_InitForOpenGL(window, true)) {
        std::cerr << "ImGui GLFW initialization failed!" << std::endl;
    } else {
        std::cout << "ImGui GLFW initialized successfully." << std::endl;
    }

    if (!ImGui_ImplOpenGL3_Init("#version 330")) {
        std::cerr << "ImGui OpenGL initialization failed!" << std::endl;
    } else {
        std::cout << "ImGui OpenGL initialized successfully." << std::endl;
    }

    ImGui::StyleColorsDark();
    std::cout << "ImGui initialization complete." << std::endl;
}


void renderImGui() {
    // Start a new frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Check if the ImGui context is active
    if (!ImGui::GetCurrentContext()) {
        std::cerr << "No active ImGui context during render!" << std::endl;
        return;
    }

    myCustomShader.useShaderProgram();

    ImVec2 windowPosition(50, 50);  // Set this to the desired position
    ImGui::SetNextWindowPos(windowPosition, ImGuiCond_Once);

    // Create UI elements
    ImGui::Begin("Lighting Controls");
    ImGui::Text("Directional Light");
    ImGui::SliderFloat3("Light Direction", &lightDir.x, -100.0f, 100.0f);
    ImGui::ColorEdit3("Light Color", &lightColor.x);

    ImGui::Separator();
    ImGui::Text("Point Light Position");
    ImGui::Text("X: %.2f", lpos[0].x);  // Show X coordinate of lpos[0]
    ImGui::Text("Y: %.2f", lpos[0].y);  // Show Y coordinate of lpos[0]
    ImGui::Text("Z: %.2f", lpos[0].z);  // Show Z coordinate of lpos[0]

    ImGui::Separator();
    ImGui::Text("Camera Position");
    ImGui::Text("X: %.2f", myCamera.getPosition().x);
    ImGui::Text("Y: %.2f", myCamera.getPosition().y);
    ImGui::Text("Z: %.2f", myCamera.getPosition().z);

    ImGui::Separator();
    ImGui::Text("Move Shelf");
    ImGui::SliderFloat3("Shelf Position", &shelfPosition.x, -100.0f, 100.0f); // Adjust range as needed
    ImGui::End();

    // Optional: Show the demo window for verification

    // Render ImGui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}




void cleanUpImGui() {
    std::cout << "Cleaning up ImGui..." << std::endl;

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    if (!ImGui::GetCurrentContext()) {
        std::cout << "ImGui context successfully destroyed." << std::endl;
    } else {
        std::cerr << "ImGui context destruction failed!" << std::endl;
    }
}


GLenum glCheckError_(const char *file, int line) {
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR)
    {
        std::string error;
        switch (errorCode)
        {
            case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
            case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
            case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
            case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
        }
        std::cout << error << " | " << file << " (" << line << ")" << std::endl;
    }
    return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
    fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);
    //TODO
}
bool isGuiMode = false;

void toggleInputMode(GLFWwindow* window) {
    if (isGuiMode) {
        // Switch to scene-navigation mode
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        isGuiMode = false;
        std::cout << "Scene-navigation mode activated." << std::endl;
    } else {
        // Switch to GUI mode
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        isGuiMode = true;
        std::cout << "GUI mode activated." << std::endl;
    }
}

bool isAnimating = false;

bool isWireframe = false; // Tracks whether wireframe mode is active


void togglePolygonMode() {
    if (isWireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Set to normal polygon mode
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Set to wireframe mode
    }
    isWireframe = !isWireframe; // Toggle the flag
}

bool dayNnite = true;

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if (key == GLFW_KEY_M && action == GLFW_PRESS)
        showDepthMap = !showDepthMap;

    if (key == GLFW_KEY_P && action == GLFW_PRESS)
        showPointMap = !showPointMap;

    if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
        toggleInputMode(window);
    }

    if (key == GLFW_KEY_N && action == GLFW_PRESS) {
        isAnimating = !isAnimating; // Toggle animation
    }

    if (key == GLFW_KEY_R && action == GLFW_PRESS) { // Toggle wireframe on 'W' key press
        togglePolygonMode();
    }

    if (key == GLFW_KEY_L && action == GLFW_PRESS) {
        dayNnite = !dayNnite;
    }

    if (key >= 0 && key < 1024)
    {
        if (action == GLFW_PRESS)
            pressedKeys[key] = true;
        else if (action == GLFW_RELEASE)
            pressedKeys[key] = false;
    }
}

float lastX = 400, lastY = 300;
float yaw = -90.0f, pitch = 0.0f;
bool firstMouse = true;

void day() {
    lightColor = glm::vec3(0.0588f, 0.0588f, 0.1019f);
    glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

    for (size_t i = 1; i < lpos.size(); ++i) {
        std::string uniformNameBase = "pointLights[" + std::to_string(i) + "]";

        glUniform3f(glGetUniformLocation(myCustomShader.shaderProgram, (uniformNameBase + ".lightColor").c_str()),
                    1.0f, 1.0f, 1.0f);
    }
//    std::string uniformNameBase = "pointLights[0]";
//
//    glUniform3f(glGetUniformLocation(myCustomShader.shaderProgram, (uniformNameBase + ".lightColor").c_str()),
//                1.0f, 0.8f, 0.6f);

    for (size_t i = 0; i < numSpotLights; i++) {
        std::string index = "spotLights[" + std::to_string(i) + "]";
        glUniform3f(glGetUniformLocation(myCustomShader.shaderProgram, (index + ".lightColor").c_str()), 1.0f, 0.8f, 0.6f);
    }
}

void night() {
    lightColor = glm::vec3(0.0f);
    glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

    for (size_t i = 1; i < lpos.size(); ++i) {
        std::string uniformNameBase = "pointLights[" + std::to_string(i) + "]";

        glUniform3f(glGetUniformLocation(myCustomShader.shaderProgram, (uniformNameBase + ".lightColor").c_str()),
                    0.0f, 0.0f, 0.0f);
    }
//    std::string uniformNameBase = "pointLights[0]";
//
//    glUniform3f(glGetUniformLocation(myCustomShader.shaderProgram, (uniformNameBase + ".lightColor").c_str()),
//                0.f, 0.f, 0.f);

    for (size_t i = 0; i < numSpotLights; i++) {
        std::string index = "spotLights[" + std::to_string(i) + "]";
        glUniform3f(glGetUniformLocation(myCustomShader.shaderProgram, (index + ".lightColor").c_str()), 0.0f, 0.0f, 0.0f);
    }
}

void updateLight() {


    view = myCamera.getViewMatrix();
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    myCustomShader.useShaderProgram();

    glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "view"),
                       1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix3fv(glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix"),
                       1, GL_FALSE, glm::value_ptr(normalMatrix));

    float time = glfwGetTime(); // Get the elapsed time
    glm::vec3 direction = glm::vec3(
            0.f,  // Vary the x-component over time
            -1.0f,      // Fixed vertical direction (downward)
            sin(time)   // Vary the z-component over time
    );

    sposEye = glm::vec3(view * glm::vec4(spos, 1.f));
//    glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "spotLight.position"), 1, glm::value_ptr(sposEye));
//    glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "spotLight.direction"), 1, glm::value_ptr(direction));

    for (int i = 0; i < numSpotLights; i++) {
        std::string index = "spotLights[" + std::to_string(i) + "]";

        sposEye = glm::vec3(view * glm::vec4(spotLightPositions[i], 1.f));


        glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, (index + ".position").c_str()), 1,
                     glm::value_ptr(sposEye));
        glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, (index + ".direction").c_str()), 1,
                     glm::value_ptr(spotLightDirections[i]));
    }



    for (size_t i = 0; i < lpos.size(); ++i) {
        // Transform light position to eye space
        glm::vec3 lposEye = glm::vec3(view * glm::vec4(lpos[i], 1.0f));

        // Use indexed uniforms to pass light data to the shader
        std::string uniformNameBase = "pointLights[" + std::to_string(i) + "]";

        // Pass position in eye space
        glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, (uniformNameBase + ".position").c_str()),
                     1, glm::value_ptr(lposEye));

        // Pass world space position
        glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, (uniformNameBase + ".wPosition").c_str()),
                     1, glm::value_ptr(lpos[i]));
    }

}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {


    if (isGuiMode)
        return;

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates range from bottom to top

    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.05f; // Adjust sensitivity as needed
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // Limit pitch values to avoid screen flipping
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    // Rotate camera based on mouse movement
    myCamera.rotate(pitch, yaw);

    updateLight();
}





void processMovement()
{
    updateLight();

    if (pressedKeys[GLFW_KEY_W]) {
        myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
        view = myCamera.getViewMatrix();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));


    }

    if (pressedKeys[GLFW_KEY_S]) {
        myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
        view = myCamera.getViewMatrix();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    }

    if (pressedKeys[GLFW_KEY_A]) {
        myCamera.move(gps::MOVE_LEFT, cameraSpeed);
        view = myCamera.getViewMatrix();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    }

    if (pressedKeys[GLFW_KEY_D]) {
        myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
        view = myCamera.getViewMatrix();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    }

    if (pressedKeys[GLFW_KEY_UP])
        lpos[0].z += 0.08;

    if (pressedKeys[GLFW_KEY_DOWN])
        lpos[0].z -= 0.08;

    if (pressedKeys[GLFW_KEY_RIGHT])
        lpos[0].x += 0.08;

    if (pressedKeys[GLFW_KEY_LEFT])
        lpos[0].x -= 0.08;

    if (pressedKeys[GLFW_KEY_LEFT_SHIFT])
        lpos[0].y -= 0.08;

    if (pressedKeys[GLFW_KEY_SPACE])
        lpos[0].y += 0.08;
}

bool initOpenGLWindow()
{
    if (!glfwInit()) {
        fprintf(stderr, "ERROR: could not start GLFW3\n");
        return false;
    }
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    //window scaling for HiDPI displays
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

    //for sRBG framebuffer
    glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);

    //for antialising
    glfwWindowHint(GLFW_SAMPLES, 4);

    glWindow = glfwCreateWindow(mode->width, mode->height, "OpenGL Shader Example", monitor, NULL);
    if (!glWindow) {
        fprintf(stderr, "ERROR: could not open window with GLFW3\n");
        glfwTerminate();
        return false;
    }

    glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
    glfwSetKeyCallback(glWindow, keyboardCallback);
    glfwSetCursorPosCallback(glWindow, mouseCallback);
    glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwMakeContextCurrent(glWindow);

    glfwSwapInterval(1);

#if not defined (_APPLE_)
    // start GLEW extension handler
    glewExperimental = GL_TRUE;
    glewInit();
#endif

    // get version info
    const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
    const GLubyte* version = glGetString(GL_VERSION); // version as a string
    printf("Renderer: %s\n", renderer);
    printf("OpenGL version supported %s\n", version);

    //for RETINA display
    glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

    return true;
}

void initOpenGLState()
{
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
    glViewport(0, 0, retina_width, retina_height);

    glEnable(GL_DEPTH_TEST); // enable depth-testing
    glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
    glEnable(GL_CULL_FACE); // cull face
    glCullFace(GL_BACK); // cull back face
    glFrontFace(GL_CCW); // GL_CCW for counter clock-wise

    glEnable(GL_FRAMEBUFFER_SRGB);
}


//gps::Model3D statue;

void initObjects() {
    ground.LoadModel("../objects/model/model.obj", myCustomShader);
    screenQuad.LoadModel("../objects/quad/quad.obj", screenQuadShader);
    //statue.LoadModel("../objects/statue/CAT2B_126final_Low.obj", myCustomShader);
}

gps::Shader waterShader;

void initShaders() {
    myCustomShader.loadShader("../shaders/shaderStart.vert", "../shaders/shaderStart.frag");
    myCustomShader.useShaderProgram();
    lightShader.loadShader("../shaders/lightCube.vert", "../shaders/lightCube.frag");
    lightShader.useShaderProgram();
    screenQuadShader.loadShader("../shaders/screenQuad.vert", "../shaders/screenQuad.frag");
    screenQuadShader.useShaderProgram();
    depthMapShader.loadShader("../shaders/depthMapShader.vert", "../shaders/depthMapShader.frag");
    depthMapShader.useShaderProgram();
    skyboxShader.loadShader("../shaders/skyboxShader.vert", "../shaders/skyboxShader.frag");
    skyboxShader.useShaderProgram();
    pointShader.loadShader2("../shaders/depthPointShader.vert", "../shaders/depthPointShader.frag", "../shaders/depthPointShader.geom");
    pointShader.useShaderProgram();
    waterShader.loadShader("../shaders/water.vert", "../shaders/water.frag");
    waterShader.useShaderProgram();
//    pointShader.loadShader("../shaders/depthPointShader.vert", "../shaders/depthPointShader.frag");
//    pointShader.useShaderProgram();
}

void initSkybox() {
    faces.push_back("../skybox/.space2/left.png");
    faces.push_back("../skybox/.space2/right.png");
    faces.push_back("../skybox/.space2/top.png");
    faces.push_back("../skybox/.space2/bottom.png");
    faces.push_back("../skybox/.space2/back.png");
    faces.push_back("../skybox/.space2/front.png");

    mySkyBox.Load(faces);

}

void initUniforms() {


    myCustomShader.useShaderProgram();

    glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "cameraPos"), 1, GL_FALSE, glm::value_ptr(myCamera.getPosition()));
    model = glm::mat4(1.0f);
    modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    view = myCamera.getViewMatrix();
    viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
    normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    projection = glm::perspective(glm::radians(90.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
    projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glUniform3f( glGetUniformLocation( myCustomShader.shaderProgram, "light.ambient" ), 0.02f, 0.02f, 0.02f );
    glUniform3f( glGetUniformLocation( myCustomShader.shaderProgram, "light.diffuse" ), 0.2f, 0.2f, 0.3f );
    glUniform3f( glGetUniformLocation( myCustomShader.shaderProgram, "light.specular" ), 0.3f, 0.3f, 0.4f );

    //set the light direction (direction towards the light)
    lightDir = glm::vec3(-27.273f, 100.0f, 100.0f);
    lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    lightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "light.direction");
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

    //set light color
    lightColor = glm::vec3(0.0588f, 0.0588f, 0.1019f);
    lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "light.lightColor");
    glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

    // Point Light
    for (size_t i = 1; i < lpos.size(); ++i) {
        // Transform light position to eye space
        glm::vec3 lposEye = glm::vec3(view * glm::vec4(lpos[i], 1.0f));

        // Use indexed uniforms for each light
        std::string uniformNameBase = "pointLights[" + std::to_string(i) + "]";

        // Set the light's position in eye space
        glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, (uniformNameBase + ".position").c_str()),
                     1, glm::value_ptr(lposEye));

        // Set the light's world position
        glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, (uniformNameBase + ".wPosition").c_str()),
                     1, glm::value_ptr(lpos[i]));

        // Set the attenuation factors
        glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, (uniformNameBase + ".constant").c_str()), constant);
        glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, (uniformNameBase + ".linear").c_str()), linear);
        glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, (uniformNameBase + ".quadratic").c_str()), quadratic);

        // Set the light's ambient, diffuse, and specular properties
        // Ambient (low base light)
        glUniform3f(glGetUniformLocation(myCustomShader.shaderProgram, (uniformNameBase + ".ambient").c_str()),
                    0.05f, 0.04f, 0.03f); // Warm color with a reddish tint

// Diffuse (main light contribution)
        glUniform3f(glGetUniformLocation(myCustomShader.shaderProgram, (uniformNameBase + ".diffuse").c_str()),
                    0.8f, 0.7f, 0.6f); // Bright and warm

// Specular (reflections)
        glUniform3f(glGetUniformLocation(myCustomShader.shaderProgram, (uniformNameBase + ".specular").c_str()),
                    1.0f, 0.9f, 0.8f); // Sharp highlights

        // Set the light color
        glUniform3f(glGetUniformLocation(myCustomShader.shaderProgram, (uniformNameBase + ".lightColor").c_str()),
                    1.0f, 1.0f, 1.0f);
    }





    glm::vec3 lposEye = glm::vec3(view * glm::vec4(lpos[0], 1.0f));

// Use indexed uniforms for each light
    std::string uniformNameBase = "pointLights[0]";

// Set the light's position in eye space
    glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, (uniformNameBase + ".position").c_str()),
                 1, glm::value_ptr(lposEye));

// Set the light's world position
    glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, (uniformNameBase + ".wPosition").c_str()),
                 1, glm::value_ptr(lpos[0]));

// Set the attenuation factors (lower reach)
    float constantRoom = 0.5f;   // Lower baseline intensity for dimmer light
    float linearRoom = 0.45f;    // Higher linear falloff for quicker intensity drop
    float quadraticRoom = 0.7f;  // Steeper quadratic falloff for very localized lighting


    glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, (uniformNameBase + ".constant").c_str()), constantRoom);
    glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, (uniformNameBase + ".linear").c_str()), linearRoom);
    glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, (uniformNameBase + ".quadratic").c_str()), quadraticRoom);

// Set the light's ambient, diffuse, and specular properties
// Ambient (low base light, warm color)
    glUniform3f(glGetUniformLocation(myCustomShader.shaderProgram, (uniformNameBase + ".ambient").c_str()),
                0.3f, 0.2f, 0.1f); // Warm light, with a reddish tone

// Diffuse (main light contribution, warmer)
    glUniform3f(glGetUniformLocation(myCustomShader.shaderProgram, (uniformNameBase + ".diffuse").c_str()),
                0.8f, 0.6f, 0.4f); // Slightly warmer and softer

// Specular (reflections)
    glUniform3f(glGetUniformLocation(myCustomShader.shaderProgram, (uniformNameBase + ".specular").c_str()),
                1.0f, 0.9f, 0.8f); // Sharp highlights, warmer reflections

// Set the light color (warmer tone)
    glUniform3f(glGetUniformLocation(myCustomShader.shaderProgram, (uniformNameBase + ".lightColor").c_str()),
                1.0f, 0.8f, 0.6f); // Light has a warm, yellowish tone

    float constantSpot = 1.0f;   // No baseline dimming
    float linearSpot = 0.06f;    // Moderate falloff
    float quadraticSpot = 0.03f; // Slightly sharper falloff to keep focus

    // Spotlight
    for (int i = 0; i < numSpotLights; i++) {
        std::string index = "spotLights[" + std::to_string(i) + "]";
        glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, (index + ".position").c_str()), 1, glm::value_ptr(spotLightPositions[i]));
        glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, (index + ".direction").c_str()), 1, glm::value_ptr(spotLightDirections[i]));
        glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, (index + ".cutOff").c_str()), spotLightCutOffs[i]);
        glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, (index + ".outerCutOff").c_str()), spotLightOuterCutOffs[i]);
        glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, (index + ".constant").c_str()), constantSpot);
        glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, (index + ".linear").c_str()), linearSpot);
        glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, (index + ".quadratic").c_str()), quadraticSpot);
        glUniform3f(glGetUniformLocation(myCustomShader.shaderProgram, (index + ".ambient").c_str()), 0.06f, 0.05f, 0.04f);
        glUniform3f(glGetUniformLocation(myCustomShader.shaderProgram, (index + ".diffuse").c_str()), 0.8f, 0.7f, 0.5f);
        glUniform3f(glGetUniformLocation(myCustomShader.shaderProgram, (index + ".specular").c_str()), 1.0f, 0.9f, 0.8f);
        glUniform3f(glGetUniformLocation(myCustomShader.shaderProgram, (index + ".lightColor").c_str()), 1.0f, 0.8f, 0.6f);
    }

    lightShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));


}



void initFBO() {
    // Generate FBO ID
    glGenFramebuffers(1, &shadowMapFBO);

    // Create depth texture
    glGenTextures(1, &depthMapTexture);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    // Attach depth texture to FBO
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);

    // Disable color buffer
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    // Unbind FBO
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}



unsigned int depthCubemap, depthMapFBO;

void initShadowPoint() {
    glGenFramebuffers(1, &depthMapFBO);


    glGenTextures(1, &depthCubemap);

    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
    for (unsigned int i = 0; i < 6; ++i)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
                     SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void setupShadowTransforms(glm::vec3 lpos1) {
    float near_plane = 1.0f;
    float far_plane = 25.0f;
    glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), (float)512 / (float)512, near_plane, far_plane);
    std::vector<glm::mat4> shadowTransforms;
    shadowTransforms.push_back(shadowProj * glm::lookAt(lpos1, lpos1 + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(lpos1, lpos1 + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(lpos1, lpos1 + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(lpos1, lpos1 + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(lpos1, lpos1 + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(lpos1, lpos1 + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));

    pointShader.useShaderProgram();
    for (unsigned int i = 0; i < 6; ++i) {
        // Create the uniform name for each shadow matrix
        std::string uniformName = "shadowMatrices[" + std::to_string(i) + "]";

        // Get the location of the uniform variable
        GLint location = glGetUniformLocation(pointShader.shaderProgram, uniformName.c_str());

        // Set the shadow matrix uniform in the shader
        glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(shadowTransforms[i]));
    }

    GLint farPlaneLocation = glGetUniformLocation(pointShader.shaderProgram, "far_plane");
    if (farPlaneLocation != -1) {
        glUniform1f(farPlaneLocation, far_plane);
    }

// Set lightPos (vec3)
    GLint lightPosLocation = glGetUniformLocation(pointShader.shaderProgram, "lightPos");
    if (lightPosLocation != -1) {
        glUniform3fv(lightPosLocation, 1, glm::value_ptr(lpos[0]));
    }

}


glm::mat4 computeLightSpaceTrMatrix() {
    // Adjust the light frustum to cover a larger area
    float frustumSize = 200.0f; // Adjust based on your scene size
    float nearPlane = 0.1f;
    float farPlane = 500.0f;

    glm::mat4 lightView = glm::lookAt(lightDir, glm::vec3(0.f), glm::vec3(.0f, 1.0f, 0.0f));
    glm::mat4 lightProjection = glm::ortho(-frustumSize, frustumSize, -frustumSize, frustumSize, nearPlane, farPlane);
    return lightProjection * lightView;
}


void drawGround(gps::Shader shader, bool depthPass) {
    model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.8f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));


    // do not send the normal matrix if we are rendering in the depth map
    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    }

    glDisable(GL_CULL_FACE);
    ground.Draw(shader);
    mySkyBox.Draw(skyboxShader, view, projection);
    glEnable(GL_CULL_FACE);


//    model = glm::translate(glm::mat4(1.0f), glm::vec3(29.19f, 1.01f, 12.18f));
//    model = glm::scale(model, glm::vec3(0.8f));
//    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
//
//
//    // do not send the normal matrix if we are rendering in the depth map
//    if (!depthPass) {
//        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
//        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
//
//    }
//
//    glDisable(GL_CULL_FACE);
//    statue.Draw(shader);
//    glEnable(GL_CULL_FACE);



}

GLuint subViewportFBO, subViewportTexture;

void initSceneFBO() {
    glGenFramebuffers(1, &subViewportFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, subViewportFBO);

// Create texture to render the subviewport scene
    glGenTextures(1, &subViewportTexture);
    glBindTexture(GL_TEXTURE_2D, subViewportTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 512, 512, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, subViewportTexture, 0);

// Create renderbuffer for depth and stencil
    GLuint rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 512, 512);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

// Check framebuffer completeness
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "Framebuffer not complete!" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void renderSubViewportScene() {
    glBindFramebuffer(GL_FRAMEBUFFER, subViewportFBO);
    glViewport(0, 0, 512, 512); // Set viewport size for the texture
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Get the time value
    float time = glfwGetTime(); // Time in seconds since the application started

    // Calculate oscillating position and target
    float amplitude = 2.0f; // Maximum left-right movement of both position and target
    float speed = 1.0f; // Speed of oscillation
    float offset = amplitude * sin(speed * time); // Sinusoidal movement

    // New camera position and target values with oscillation
    glm::vec3 subCameraPos = glm::vec3(69.43f , 5.51f, -9.20f); // Camera position oscillates along X axis
    glm::vec3 subCameraTarget = glm::vec3(59.22f + offset, 2.66f, 1.67); // Target oscillates along X axis
    glm::mat4 subViewMatrix = glm::lookAt(subCameraPos, subCameraTarget, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 subProjectionMatrix = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);

    myCustomShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(subViewMatrix));
    glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // Render the subviewport scene
    drawGround(myCustomShader, true);
    mySkyBox.Draw(skyboxShader, subViewMatrix, subProjectionMatrix);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


//29.22, 0.22, 11.02
//24.16, 2.40, 3.93
//15 15 26



void renderScene() {
    renderSubViewportScene();

    depthMapShader.useShaderProgram();
    // Depth map creation pass
    // Set the light-space transformation matrix uniform
    glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"),
                       1,
                       GL_FALSE,
                       glm::value_ptr(computeLightSpaceTrMatrix()));

    // Configure viewport for depth map rendering
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);

    // Bind the framebuffer and clear the depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    // Render the scene into the depth buffer
    drawGround(depthMapShader, true);
    // Unbind the framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);



        pointShader.useShaderProgram();
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        setupShadowTransforms(lpos[0]);
        drawGround(pointShader, true);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);


     //Rendering pass with shadows
// Use the custom shader for the scene

    myCustomShader.useShaderProgram();

    glViewport(0, 0, retina_width, retina_height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClear(GL_DEPTH_BUFFER_BIT);
    view = myCamera.getViewMatrix();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model)    );
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

    glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "light.lightColor"), 1, glm::value_ptr(lightColor));

    if (!dayNnite)
        night();
    else day();

// Bind the shadow map
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 3);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
    glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "depthMap"), 4);

    glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"),
                       1,
                       GL_FALSE,
                       glm::value_ptr(computeLightSpaceTrMatrix()));

    glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "cameraPos"), 1,glm::value_ptr(myCamera.getPosition()));


// Draw scene objects
    drawGround(myCustomShader, false);

// Draw the skybox
    mySkyBox.Draw(skyboxShader, view, projection);

// Render the depth map quad in the scene
    screenQuadShader.useShaderProgram();
    glEnable(GL_DEPTH_TEST);
// Bind the depth map texture
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, subViewportTexture);
    glUniform1i(glGetUniformLocation(screenQuadShader.shaderProgram, "depthMap"), 5);

// Disable depth testing to ensure the quad is always drawn on top of other objects

// Set the position and scale of the quad within the scene
    auto quadModel = glm::mat4(1.0f);
    quadModel = glm::translate(quadModel, glm::vec3(29.15, 0.14, 11.74));  // Position the quad in the scene
    quadModel = glm::scale(quadModel, glm::vec3(.14f, .10f, .10f));  // Adjust size
    glUniformMatrix4fv(glGetUniformLocation(screenQuadShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(quadModel));
    glUniformMatrix4fv(glGetUniformLocation(screenQuadShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(screenQuadShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));


// Render the quad
    screenQuad.Draw(screenQuadShader);



}

void cleanup() {
    glDeleteTextures(1,& depthMapTexture);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &shadowMapFBO);
    glfwDestroyWindow(glWindow);
    //close GL context and any other GLFW resources
    glfwTerminate();
}

bool initOpenAL() {
    ALCdevice* device = alcOpenDevice(nullptr); // Open the default device
    if (!device) {
        std::cerr << "Failed to open OpenAL device" << std::endl;
        return false;
    }

    ALCcontext* context = alcCreateContext(device, nullptr); // Create context
    if (!context) {
        std::cerr << "Failed to create OpenAL context" << std::endl;
        return false;
    }

    alcMakeContextCurrent(context); // Set current context

    return true;
}

// Load and decode an MP3 file using libsndfile, then send the data to OpenAL
ALuint loadMP3(const std::string& filePath) {
    SF_INFO fileInfo;
    SNDFILE* file = sf_open(filePath.c_str(), SFM_READ, &fileInfo);
    if (!file) {
        std::cerr << "Failed to open MP3 file: " << sf_strerror(file) << std::endl;
        return 0;
    }

    // Read the MP3 data
    auto* buffer = new short[fileInfo.frames * fileInfo.channels];
    sf_read_short(file, buffer, fileInfo.frames * fileInfo.channels);
    sf_close(file);

    // Create OpenAL buffer and send the decoded data to OpenAL
    ALuint alBuffer;
    alGenBuffers(1, &alBuffer);
    alBufferData(alBuffer, AL_FORMAT_STEREO16, buffer, fileInfo.frames * fileInfo.channels * sizeof(short), fileInfo.samplerate);

    delete[] buffer;
    return alBuffer;
}

// Play the MP3 using OpenAL
ALuint playMP3(ALuint alBuffer) {
    ALuint source;
    alGenSources(1, &source);
    if (alGetError() != AL_NO_ERROR) {
        std::cerr << "Failed to generate OpenAL source" << std::endl;
        return 0;
    }

    alSourcei(source, AL_BUFFER, alBuffer);
    alSourcePlay(source);

    return source;  // Return the source to be used in the main loop
}


int main(int argc, const char* argv[]) {
    // Initialize OpenGL window first
    if (!initOpenGLWindow()) {
        std::cerr << "Failed to initialize OpenGL window" << std::endl;
        return 1;
    }

    if (!initOpenAL()) {
        std::cerr << "OpenAL initialization failed" << std::endl;
        return 1;
    }

    // Load the MP3 file (ensure you have a valid file path)
    ALuint alBuffer = loadMP3("../luxor.mp3");
    if (alBuffer == 0) {
        std::cerr << "Failed to load MP3 file" << std::endl;
        return 1;
    }

    // Play the MP3 asynchronously and get the OpenAL source

    initOpenGLState();
    initShaders();
    initObjects();
    initUniforms();
    initShadowPoint();
    initSceneFBO();
    initFBO();
    initSkybox();
    initImGui(glWindow);

    std::vector<Keyframe> keyframes = createKeyframes();

    // Timing variables
    float currentTime = 0.0f;
    float lastFrameTime = 0.0f;

    glCheckError();
    ALuint source = playMP3(alBuffer);
    if (source == 0) {
        std::cerr << "Failed to play MP3" << std::endl;
        return 1;
    }

    // Main render loop
    while (!glfwWindowShouldClose(glWindow)) {
        myCustomShader.useShaderProgram();
        float currentFrameTime = glfwGetTime();
        glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "uTime"), currentFrameTime);

        float deltaTime = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;

        float scaledDeltaTime = deltaTime * 0.5f; // Scale deltaTime for slower movement

        // Update current time
        currentTime += deltaTime;

        // Update camera movement and rotation
        if (isAnimating) {
            moveAndRotateCamera(keyframes, currentTime, scaledDeltaTime);
        }

        processMovement();
        renderScene();
        renderImGui();

        // Check the state of the source to see if it's still playing
        ALint state;
        alGetSourcei(source, AL_SOURCE_STATE, &state);
        if (state == AL_STOPPED) {
            alSourcePlay(source);  // Restart playback if needed
        }

        glfwPollEvents();
        glfwSwapBuffers(glWindow);
    }

    alDeleteSources(1, &source);
    alDeleteBuffers(1, &alBuffer);

    // Properly close OpenAL
    ALCcontext* context = alcGetCurrentContext();
    ALCdevice* device = alcGetContextsDevice(context);
    alcMakeContextCurrent(nullptr);
    alcDestroyContext(context);
    alcCloseDevice(device);
    // Cleanup OpenGL and OpenAL resources
    cleanup();
    cleanUpImGui();


    return 0;
}
