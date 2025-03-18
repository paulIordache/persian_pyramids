//
//  main.cpp
//  OpenGL Advances Lighting
//
//  Created by CGIS on 28/11/16.
//  Copyright � 2016 CGIS. All rights reserved.
//

#if defined (__APPLE__)
    #define GLFW_INCLUDE_GLCOREARB
    #define GL_SILENCE_DEPRECATION
#else
    #define GLEW_STATIC
    #include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.hpp"
#include "Model3D.hpp"
#include "Camera.hpp"

#include <iostream>

#include "SkyBox.hpp"


gps::SkyBox skyboxD, skyboxS, skyboxR;

int glWindowWidth = 1900;
int glWindowHeight = 2000;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;

const unsigned int SHADOW_WIDTH = 2048;
const unsigned int SHADOW_HEIGHT = 2048;

glm::mat4 model;
GLuint modelLoc;
glm::mat4 view;
GLuint viewLoc;
glm::mat4 projection;
GLuint projectionLoc;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;
glm::mat4 lightRotation;
GLint fog;
GLint fogSky;


glm::vec3 lightDir;
GLuint lightDirLoc;
glm::vec3 lightColor;
GLuint lightColorLoc;

GLuint rainNormalMatrixLoc;
GLuint snowNormalMatrixLoc;
GLuint ironmanNormalMatrixLoc;

glm::mat4 snowModel;
glm::mat3 snowNormalMatrix;

glm::mat4 rainModel;
glm::mat3 rainNormalMatrix;

gps::Camera myCamera(
				glm::vec3(-10.0f, 10.0f, -5.0f),
				glm::vec3(0.0f, -10.0f, 0.0f),
				glm::vec3(0.0f, 1.0f, 0.0f));
float cameraSpeed = 0.07f;
// Camera class member variables
float lastX = 400.0f;  // initial position of the mouse (half the window width)
float lastY = 300.0f;  // initial position of the mouse (half the window height)
float pitch = 0.0f;
float yaw = -90.0f;  // starting yaw (pointing in negative Z direction)
bool firstMouse = true; // flag to detect the first mouse movement

int sunSnowRain = 1;
bool pressedKeys[1024];
float suny = -1;
float sunx=3.5;
bool bNoapte = false;
bool bFog = false;
float fogDensity = 0;

float angleY = 0.0f;
GLfloat lightAngle;

gps::Model3D ground;
gps::Model3D lightCube;
gps::Model3D screenQuad;
gps::SkyBox mySkyBox;

gps::Model3D city;
gps::Model3D sun;
gps::Model3D snow;
gps::Model3D rain;

gps::Shader myCustomShader;
gps::Shader lightShader;
gps::Shader screenQuadShader;
gps::Shader skyboxShader;
gps::Shader depthMapShader;
gps::Shader spotLightShader;

GLuint shadowMapFBO;
GLuint depthMapTexture;

bool showDepthMap;
float snowAux = 70.0f;
float snowSpeed = 0.5f;
float rainAux = 70.0f;
float rainSpeed = 1.0f;
int control = 1;

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
	fprintf(stdout, "window resized to width: %d, and height: %d\n", width, height);
	if (height == 0) height = 1; // Prevent division by zero
	glViewport(0, 0, width, height);

	// Update the perspective projection matrix
	projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	myCustomShader.useShaderProgram();
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
}


void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_M && action == GLFW_PRESS)
		showDepthMap = !showDepthMap;

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			pressedKeys[key] = true;
		else if (action == GLFW_RELEASE)
			pressedKeys[key] = false;
	}
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {

	myCamera.processMouseMovement(xpos,ypos);
}


void processMovement()
{
	view = myCamera.getViewMatrix();
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	if (pressedKeys[GLFW_KEY_O]) {
		lightDir += glm::vec3(0.01f, 0.0f, 0.0f);
	}
	if (pressedKeys[GLFW_KEY_P]) {
		lightDir -= glm::vec3(0.01f, 0.0f, 0.0f);
	}
	if (pressedKeys[GLFW_KEY_Y]) {
		lightDir -= glm::vec3(0.0f, 0.0f, 0.01f);
	}
	if (pressedKeys[GLFW_KEY_U]) {
		lightDir += glm::vec3(0.0f, 0.0f, 0.01f);
	}


	if (pressedKeys[GLFW_KEY_Q]) {
		angleY -= 1.0f;
	}

	if (pressedKeys[GLFW_KEY_E]) {
		angleY += 1.0f;
	}

	if (pressedKeys[GLFW_KEY_J]) {
		lightAngle -= 1.0f;
	}

	if (pressedKeys[GLFW_KEY_L]) {
		lightAngle += 1.0f;
	}

	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
	}

	// Add the new movement keys for ascend and descend
	if (pressedKeys[GLFW_KEY_SPACE]) { // Ascend (move up)
		myCamera.move(gps::MOVE_UP, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_LEFT_CONTROL]) { // Descend (move down)
		myCamera.move(gps::MOVE_DOWN, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_UP]) { // Move the camera up (for example, on the Y axis)
		myCamera.move(gps::MOVE_UP, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_DOWN]) { // Move the camera down (for example, on the Y axis)
		myCamera.move(gps::MOVE_DOWN, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_LEFT_SHIFT]) { // Move the camera down (for example, on the Y axis)
		myCamera.move(gps::MOVE_FORWARD, 10*cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_M])
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	if (pressedKeys[GLFW_KEY_N])
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
	}
	if (pressedKeys[GLFW_KEY_B])
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	if(pressedKeys[GLFW_KEY_Z]) {

		mySkyBox = skyboxD;
		sunSnowRain = 1;
	}

	if(pressedKeys[GLFW_KEY_X]) {

		mySkyBox = skyboxR;
		sunSnowRain = 2;
	}

	if(pressedKeys[GLFW_KEY_C]) {

		mySkyBox = skyboxS;
		sunSnowRain = 3;
	}

	if (pressedKeys[GLFW_KEY_LEFT]) { // Rotate camera to the left
		myCamera.rotate(myCamera.pitch, myCamera.yaw - 1.0f); // Negative yaw for left rotation
	}

	if (pressedKeys[GLFW_KEY_RIGHT]) { // Rotate camera to the right
		myCamera.rotate(myCamera.pitch, myCamera.yaw + 1.0f); // Positive yaw for right rotation
	}


	if (pressedKeys[GLFW_KEY_I])
	{
		bNoapte = true;
	}
	else if (bNoapte)
	{
		myCustomShader.useShaderProgram();

		switch (control) {

			case 1:
				lightColor = glm::vec3(false);
			lightColor += glm::vec3(0.03f, 0.03f, 0.3f) * (1.0f - false);
			glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightColor"), 1, glm::value_ptr(lightColor));
			control = 2;
			break;
			case 2:
				lightColor = glm::vec3(false);
			lightColor += glm::vec3(0.03f, 0.2f, 0.03f) * (1.0f - false);
			glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightColor"), 1, glm::value_ptr(lightColor));
			control = 3;
			break;
			case 3:
				lightColor = glm::vec3(false);
			lightColor += glm::vec3(0.2f, 0.03f, 0.03f) * (1.0f - false);
			glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightColor"), 1, glm::value_ptr(lightColor));
			control = 4;
			break;
			case 4:
				lightColor = glm::vec3(true);
			lightColor += glm::vec3(0.03f, 0.03f, 0.3f) * (1.0f - true);
			glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightColor"), 1, glm::value_ptr(lightColor));
			control = 1;
			break;
			default:
				break;
		}

		bNoapte = false;
	}


	if (pressedKeys[GLFW_KEY_F])
	{
		bFog = true;
	}
	else
	{
		if (bFog)
		{
			fogDensity = (fogDensity + 0.015) * (fogDensity <= 0.05);
			myCustomShader.useShaderProgram();
			fog = glGetUniformLocation(myCustomShader.shaderProgram, "fogDensity");
			glUniform1f(fog, fogDensity);
			fogSky = glGetUniformLocation(skyboxShader.shaderProgram, "fogDensity");;
			glUniform1f(fogSky, fogDensity);


		}
		bFog = false;
	}

}


void initSkyBox()
{
	std::vector<const GLchar*> face1;
	face1.push_back("../skybox/Day/posx.jpg");
	face1.push_back("../skybox/Day/negx.jpg");
	face1.push_back("../skybox/Day/posy.jpg");
	face1.push_back("../skybox/Day/negy.jpg");
	face1.push_back("../skybox/Day/posz.jpg");
	face1.push_back("../skybox/Day/negz.jpg");

	skyboxD.Load(face1);
	mySkyBox = skyboxD;

	std::vector<const GLchar*> face2;
	face2.push_back("../skybox/Clouds/left.png");
	face2.push_back("../skybox/Clouds/right.png");
	face2.push_back("../skybox/Clouds/top.png");
	face2.push_back("../skybox/Clouds/bottom.png");
	face2.push_back("../skybox/Clouds/front.png");
	face2.push_back("../skybox/Clouds/back.png");

	skyboxR.Load(face2);

	std::vector<const GLchar*> face3;
	face3.push_back("../skybox/Snow/posx.jpg");
	face3.push_back("../skybox/Snow/negx.jpg");
	face3.push_back("../skybox/Snow/posy.jpg");
	face3.push_back("../skybox/Snow/negy.jpg");
	face3.push_back("../skybox/Snow/posz.jpg");
	face3.push_back("../skybox/Snow/negz.jpg");

	skyboxS.Load(face3);
}

void initSkyBoxShader()
{
	skyboxShader.loadShader("../shaders/skyboxShader.vert", "../shaders/skyboxShader.frag");
	skyboxShader.useShaderProgram();
	view = myCamera.getViewMatrix();
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "view"), 1, GL_FALSE,
		glm::value_ptr(view));

	projection = glm::perspective(glm::radians(90.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "projection"), 1, GL_FALSE,
		glm::value_ptr(projection));


	glUniform1f(glGetUniformLocation(skyboxShader.shaderProgram, "fogDensity"), fogDensity);
}


bool initOpenGLWindow()
{
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

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

	glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "OpenGL Shader Example", NULL, NULL);
	if (!glWindow) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}

	glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
	glfwSetKeyCallback(glWindow, keyboardCallback);
	glfwSetCursorPosCallback(glWindow, mouseCallback);
	//glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwMakeContextCurrent(glWindow);

	glfwSwapInterval(1);

#if not defined (__APPLE__)
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

void initObjects() {


	city.LoadModel("../objects/city/city.obj", myCustomShader);
}

void initShaders() {
	myCustomShader.loadShader("../shaders/shaderStart.vert", "../shaders/shaderStart.frag");
	myCustomShader.useShaderProgram();
	screenQuadShader.loadShader("../shaders/screenQuad.vert", "../shaders/screenQuad.frag");
	screenQuadShader.useShaderProgram();
	depthMapShader.loadShader("../shaders/depthMapShader.vert", "../shaders/depthMapShader.frag");
	depthMapShader.useShaderProgram();
}

void initUniforms() {

	myCustomShader.useShaderProgram();

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
	glUniformMatrix4fv(projectionLoc, 1,  GL_FALSE, glm::value_ptr(projection));

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(1.0f, 1.0f, 0.0f);
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	glm::vec3 ambient = glm::vec3(0.1f, 0.1f, 0.1f);   // Soft ambient light
	glm::vec3 diffuse = glm::vec3(0.8f, 0.8f, 0.8f);   // Stronger diffuse light
	glm::vec3 specular = glm::vec3(1.0f, 1.0f, 1.0f);  // Bright specular highlight

	// Send the uniform data
	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "light.direction"), 1, glm::value_ptr(lightDir));
	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "light.lightColor"), 1, glm::value_ptr(lightColor));
	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "light.ambient"), 1, glm::value_ptr(ambient));
	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "light.diffuse"), 1, glm::value_ptr(diffuse));
	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "light.specular"), 1, glm::value_ptr(specular));

	// Set the light's position
	glm::vec3 position = glm::vec3(0.0f, 5.0f, 0.0f);       // Position of the spotlight
	glm::vec3 wPosition = glm::vec3(0.0f, 5.0f, 0.0f);      // World position of the spotlight

	// Set the light's direction
	glm:: vec3 direction = glm::vec3(0.0f, -1.0f, 0.0f);     // Direction the spotlight is pointing

	// Set the cutoff angles
	float cutOff = glm::cos(glm::radians(12.5f));       // Inner cutoff (spotlight cone angle)
	float outerCutOff = glm::cos(glm::radians(17.5f));  // Outer cutoff (soft edge of the spotlight)

	// Set attenuation parameters
	float constant = 1.0f;                              // Constant attenuation factor
	float linear = 0.09f;                               // Linear attenuation factor
	float quadratic = 0.032f;                           // Quadratic attenuation factor

	// Set the light color and intensities
	glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);     // White light
	ambient = glm::vec3(0.1f, 0.1f, 0.1f);        // Soft ambient light
	diffuse = glm::vec3(0.8f, 0.8f, 0.8f);        // Stronger diffuse light
	specular = glm::vec3(1.0f, 1.0f, 1.0f);       // Bright specular highlights


	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "spotLight.position"), 1, glm::value_ptr(position));
	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "spotLight.wPosition"), 1, glm::value_ptr(wPosition));
	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "spotLight.direction"), 1, glm::value_ptr(direction));
	glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "spotLight.cutOff"), cutOff);
	glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "spotLight.outerCutOff"), outerCutOff);
	glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "spotLight.constant"), constant);
	glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "spotLight.linear"), linear);
	glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "spotLight.quadratic"), quadratic);
	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "spotLight.lightColor"), 1, glm::value_ptr(color));
	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "spotLight.ambient"), 1, glm::value_ptr(ambient));
	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "spotLight.diffuse"), 1, glm::value_ptr(diffuse));
	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "spotLight.specular"), 1, glm::value_ptr(specular));

	//fog = glGetUniformLocation(myCustomShader.shaderProgram, "fogDensity");
	//glUniform1f(fog, (GLfloat)fogDensity);
}

void initFBO() {
	glGenFramebuffers(1, &shadowMapFBO);

	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
				 SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);
	glDrawBuffer(GL_NONE); // No color buffer is drawn to.
	glReadBuffer(GL_NONE);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cerr << "Framebuffer not complete!" << std::endl;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

glm::mat4 computeLightSpaceTrMatrix() {
	glm::mat4 orthgonalProjection = glm::ortho(-35.0f, 35.0f, -35.0f, 35.0f, 0.1f, 75.0f);
	glm::mat4 lightView = glm::lookAt(20.0f * lightDir, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 lightProjection = orthgonalProjection * lightView;

	return lightProjection;
}

glm::mat4 computePerspectiveProjection() {

	glm::mat4 perspectiveProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 100.0f);
	glm::mat4 lightView = glm::lookAt( glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	glm::mat4 lightProjection = perspectiveProjection * lightView;

	return lightProjection;
}

void drawObjects(gps::Shader shader, bool depthPass) {

	shader.useShaderProgram();

	model = glm::mat4(1.0f);
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	// do not send the normal matrix if we are rendering in the depth map
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	city.Draw(shader);

	if (sunSnowRain == 3) {
		snowModel = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, snowAux, 0.0f));
		glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(snowModel));
		if (!depthPass) {
			snowNormalMatrix = glm::mat3(glm::inverseTranspose(view * snowModel));
			glUniformMatrix3fv(snowNormalMatrixLoc, 1, GL_FALSE, glm::value_ptr(snowNormalMatrix));
		}

		snow.Draw(shader);
	}

	if (sunSnowRain == 2) {
		rainModel = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, rainAux, 0.0f));
		glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(rainModel));
		if (!depthPass) {
			rainNormalMatrix = glm::mat3(glm::inverseTranspose(view * rainModel));
			glUniformMatrix3fv(rainNormalMatrixLoc, 1, GL_FALSE, glm::value_ptr(rainNormalMatrix));
		}

		rain.Draw(shader);
	}

}

void renderScene() {


	depthMapShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"), 1, GL_FALSE, glm::value_ptr(computeLightSpaceTrMatrix()));
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	drawObjects(depthMapShader, 1);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	if (sunSnowRain == 1)
	{
		if (suny < 27.4 && sunx < 22.9)
		{
			//lightDir += glm::vec3(0.01f, 0.01f, 0.0f);
			suny += 0.01;
			sunx += 0.01;
		}

		if (sunx >= 22)
		{
			//lightDir += glm::vec3(0.01f, -0.01f, 0.0f);
			suny -= 0.01;
			sunx += 0.01;
		}
	}
	if (sunSnowRain == 3)
	{
		if (snowAux > -70.0)
			snowAux -= snowSpeed;
	}
	if (sunSnowRain == 2)
	{
		if (rainAux > -70.0)
			rainAux -= rainSpeed;
	}


	// render depth map on screen - toggled with the M key

	if (showDepthMap) {
		glViewport(0, 0, retina_width, retina_height);

		glClear(GL_COLOR_BUFFER_BIT);

		screenQuadShader.useShaderProgram();

		//bind the depth map
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(screenQuadShader.shaderProgram, "depthMap"), 0);

		glDisable(GL_DEPTH_TEST);
		screenQuad.Draw(screenQuadShader);
		glEnable(GL_DEPTH_TEST);
	}
	else {
		// final scene rendering pass (with shadows)

		glViewport(0, 0, retina_width, retina_height);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		myCustomShader.useShaderProgram();

		view = myCamera.getViewMatrix();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

		//lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
		//glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

		//bind the shadow map
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 3);

		glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"),
			1,
			GL_FALSE,
			glm::value_ptr(computeLightSpaceTrMatrix()));

		glm::vec3 cameraPos = myCamera.cameraPosition; // Your camera position
		glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "cameraPos"), 1, glm::value_ptr(cameraPos));

		drawObjects(myCustomShader, false);


		//skyboxShader.useShaderProgram();

		//glm::vec3 cameraPos = myCamera.cameraPosition; // Your camera position
		//glUniform3fv(glGetUniformLocation(skyboxShader.shaderProgram, "cameraPos"), 1, glm::value_ptr(cameraPos));

		// Render the skybox after all other objects
		//mySkyBox.Draw(skyboxShader, view, projection);
	}
}

void cleanup() {
	glDeleteTextures(1,& depthMapTexture);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &shadowMapFBO);
	glfwDestroyWindow(glWindow);
	//close GL context and any other GLFW resources
	glfwTerminate();
}

int main(int argc, const char * argv[]) {

	if (!initOpenGLWindow()) {
		glfwTerminate();
		return 1;
	}

	initOpenGLState();
	initObjects();
	initShaders();
	initUniforms();
	initFBO();
	initSkyBox();
	initSkyBoxShader();

	glCheckError();

	while (!glfwWindowShouldClose(glWindow)) {
		processMovement();
		renderScene();

		glfwPollEvents();
		glfwSwapBuffers(glWindow);
	}

	cleanup();

	return 0;
}
