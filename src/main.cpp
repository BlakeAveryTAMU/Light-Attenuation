#include <cassert>
#include <cstring>
#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "Camera.h"
#include "GLSL.h"
#include "MatrixStack.h"
#include "Program.h"
#include "Shape.h"
#include "Material.h"
#include "Light.h"
#include "Object.h"
#include "FreeLookCamera.h"
#include "Texture.h"
#include <random>

using namespace std;

GLFWwindow *window; // Main application window
string RESOURCE_DIR = "./"; // Where the resources are loaded from
bool OFFLINE = false;

shared_ptr<Texture> texture0;
shared_ptr<Camera> camera;
shared_ptr<FreeLookCamera> freeCam;
shared_ptr<Program> prog;
shared_ptr<Program> prog2;
shared_ptr<Program> prog3;
shared_ptr<Program> prog4;
shared_ptr<Shape> bunny;
shared_ptr<Shape> teapot;
shared_ptr<Shape> plane;
shared_ptr<Shape> sun;
shared_ptr<Shape> frustum;

float minYTeapot;
float minYBunny;
float minYCube;

int activated = 0;

vector<shared_ptr<Program>> programs;
shared_ptr<Program> currProgram;
int progIndex = 0;

vector<Material> materials;
Material currMaterial;
int matIndex = 0;

vector<Light> lights;
Light* currLight;
int lightIndex = 0;

vector<Object*> objects;
Object* currObject;

bool keyToggles[256] = {false}; // only for English keyboards!

// This function is called when a GLFW error occurs
static void error_callback(int error, const char *description)
{
	cerr << description << endl;
}

// This function is called when a key is pressed
static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
}


// This function is called when the mouse is clicked
static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
	// Get the current mouse position.
	double xmouse, ymouse;
	glfwGetCursorPos(window, &xmouse, &ymouse);
	// Get current window size.
	int width, height;
	glfwGetWindowSize(window, &width, &height);
	if(action == GLFW_PRESS) {
		bool shift = (mods & GLFW_MOD_SHIFT) != 0;
		bool ctrl  = (mods & GLFW_MOD_CONTROL) != 0;
		bool alt   = (mods & GLFW_MOD_ALT) != 0;
		//camera->mouseClicked((float)xmouse, (float)ymouse, shift, ctrl, alt);
		camera->mouseClicked((float)xmouse, (float)ymouse, shift, ctrl, alt);
	}
}

// This function is called when the mouse moves
static void cursor_position_callback(GLFWwindow* window, double xmouse, double ymouse)
{
	int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
	if(state == GLFW_PRESS) {
		//camera->mouseMoved((float)xmouse, (float)ymouse);
		camera->mouseMoved((float)xmouse, (float)ymouse);
	}
}

static void char_callback(GLFWwindow *window, unsigned int key)
{
	/*switch (key) {

		case 'w':
			freeCam->keyPressed(key);
			break;
		case 'a':
			freeCam->keyPressed(key);
			break;
		case 's':
			freeCam->keyPressed(key);
			break;
		case 'd':
			freeCam->keyPressed(key);
			break;
		case 'z':
			freeCam->updateFOV(key);
			break;
		case 'Z':
			freeCam->updateFOV(key);
			break;
		case 't':
			activated += 1;
			break;
	
	}*/

}

// If the window is resized, capture the new size and reset the viewport
static void resize_callback(GLFWwindow *window, int width, int height)
{
	glViewport(0, 0, width, height);
}

// https://lencerf.github.io/post/2019-09-21-save-the-opengl-rendering-to-image-file/
static void saveImage(const char *filepath, GLFWwindow *w)
{
	int width, height;
	glfwGetFramebufferSize(w, &width, &height);
	GLsizei nrChannels = 3;
	GLsizei stride = nrChannels * width;
	stride += (stride % 4) ? (4 - stride % 4) : 0;
	GLsizei bufferSize = stride * height;
	std::vector<char> buffer(bufferSize);
	glPixelStorei(GL_PACK_ALIGNMENT, 4);
	glReadBuffer(GL_BACK);
	glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, buffer.data());
	stbi_flip_vertically_on_write(true);
	int rc = stbi_write_png(filepath, width, height, nrChannels, buffer.data(), stride);
	if(rc) {
		cout << "Wrote to " << filepath << endl;
	} else {
		cout << "Couldn't write to " << filepath << endl;
	}
}

// This function is called once to initialize the scene and OpenGL
static void init()
{
	// Initialize time.
	glfwSetTime(0.0);
	
	// Set background color.
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	// Enable z-buffer test.
	glEnable(GL_DEPTH_TEST);

	currProgram = make_shared<Program>();


	//Blinn-Phong Shader
	prog2 = make_shared<Program>();
	prog2->setShaderNames(RESOURCE_DIR + "vert.glsl", RESOURCE_DIR + "frag.glsl");
	prog2->setVerbose(true);
	prog2->init();
	prog2->addAttribute("aPos");
	prog2->addAttribute("aNor");
	prog2->addUniform("MV");
	prog2->addUniform("P");
	prog2->addUniform("lightPos1");
	prog2->addUniform("lightColor1");
	prog2->addUniform("ka");
	prog2->addUniform("kd");
	prog2->addUniform("ks");
	prog2->addUniform("s");
	prog2->setVerbose(false);
	prog2->addUniform("MVit");
	programs.push_back(prog2);



	Material m1;
	m1.setAmbient({ 0.2f, 0.2f, 0.2f });
	m1.setDiffuse({ 1.0f, 0.0f, 0.0f });
	m1.setSpecular({ 1.0f, 0.9f, 0.8f });
	m1.setShiny(200.0f);
	materials.push_back(m1);

	Material m2;
	m2.setAmbient({ 0.2f, 0.2f, 0.2f });
	m2.setDiffuse({ 0.0f, 0.0f, 0.8f });
	m2.setSpecular({ 0.0f, 0.9f, 0.0f });
	m2.setShiny(200.0f);
	materials.push_back(m2);

	Material m3;
	m3.setAmbient({ 0.1f, 0.1f, 0.1f });
	m3.setDiffuse({ 0.2f, 0.2f, 0.2f });
	m3.setSpecular({ 0.3f, 0.3f, 0.45f });
	m3.setShiny(2.0f);
	materials.push_back(m3);

	Light l1;
	l1.setPosition({ 5.0f, 2.0f, 3.0f });
	l1.setColor({ 1.0f, 1.0f, 1.0f });
	lights.push_back(l1);


	//set default program
	currProgram = programs[0];
	//set the default material
	currMaterial = materials[0];
	//set the default light
	currLight = &lights[0];

	camera = make_shared<Camera>();
	camera->setInitDistance(2.0f); // Camera's initial Z translation

	freeCam = make_shared<FreeLookCamera>();
	freeCam->setInitDistance(2.0f);
	
	bunny = make_shared<Shape>();
	bunny->loadMesh(RESOURCE_DIR + "bunny.obj");
	bunny->init();

	teapot = make_shared<Shape>();
	teapot->loadMesh(RESOURCE_DIR + "teapot.obj");
	teapot->init();

	plane = make_shared<Shape>();
	plane->loadMesh(RESOURCE_DIR + "square.obj");
	minYCube = plane->getMinY();
	plane->init();

	sun = make_shared<Shape>();
	sun->loadMesh(RESOURCE_DIR + "sphere2.obj");
	sun->init();

	frustum = make_shared<Shape>();
	frustum->loadMesh(RESOURCE_DIR + "frustum.obj");
	frustum->init();


	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 10; j++) {

			Object* obj = new Object();
			if (j % 2 == 0) {
				obj->setShape(bunny); // bunny
				obj->setTranslation(glm::vec3(j, -0.066618, i));
				//obj->setScale(glm::vec3(0.2, 0.2, 0.2));

			}
			else {
				obj->setShape(teapot); // teapot
				obj->setTranslation(glm::vec3(j, 0, i));
				//obj->setScale(glm::vec3(0.2, 0.2, 0.2));
			}

			objects.push_back(obj);
		}
	}
	currObject = objects[0];

	Object* last_bunny = new Object();
	last_bunny->setShape(bunny);
	last_bunny->setTranslation(glm::vec3(1, -0.066618, -3));
	last_bunny->setScale(glm::vec3(0.2, 0.2, 0.2));
	objects.push_back(last_bunny);
	
	GLSL::checkError(GET_FILE_LINE);
}

// This function is called every frame to draw the scene.
static void render()
{
	// Clear framebuffer.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (keyToggles[(unsigned)'c']) {
		glEnable(GL_CULL_FACE);
	}
	else {
		glDisable(GL_CULL_FACE);
	}
	/*if (keyToggles[(unsigned)'z']) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}*/

	// Get current frame buffer size.
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	camera->setAspect((float)width / (float)height);
	//reeCam->setAspect((float)width / (float)height);

	float aspect_ratio = (float)width / (float)height;

	double t = glfwGetTime();
	

	// Matrix stacks
	auto P = make_shared<MatrixStack>();
	auto MV = make_shared<MatrixStack>();

	glViewport(0, 0, width, height);
	// Apply camera transforms
	P->pushMatrix();
	
	//freeCam->applyProjectionMatrix(P);
	camera->applyProjectionMatrix(P);
	MV->pushMatrix();
	//freeCam->applyViewMatrix(MV);
	camera->applyViewMatrix(MV);
	
	//Draw Sun --------------------------------------------------------------------------------------
	glm::vec3 temp = MV->topMatrix() * glm::vec4(lights[0].getPosition(), 1);

	MV->pushMatrix();
	{
		MV->translate(lights[0].getPosition());
		MV->scale(0.2, 0.2, 0.2);

		prog2->bind();
		glUniform3f(prog2->getUniform("lightPos1"), temp[0], temp[1], temp[2]);
		glUniform3f(prog2->getUniform("lightColor1"), lights[0].getColor()[0], lights[0].getColor()[1], lights[0].getColor()[2]);
		glUniformMatrix4fv(prog2->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
		glUniformMatrix4fv(prog2->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
		glUniformMatrix4fv(prog2->getUniform("MVit"), 1, GL_FALSE, glm::value_ptr(transpose(inverse(MV->topMatrix()))));
		glUniform3f(prog2->getUniform("ka"), 1.0f, 1.0, 0);
		glUniform3f(prog2->getUniform("kd"), 0, 0, 0);
		glUniform3f(prog2->getUniform("ks"), 0, 0, 0);
		glUniform1f(prog2->getUniform("s"), currMaterial.getShiny());
		sun->draw(prog2);
		prog2->unbind();

	}
	MV->popMatrix();
	// ---------------------------------------------------------------------------------------------------

	glm::mat4 S(1.0f);
	S[0][1] = 0.5f * cos(t);

	//Draw Ground
	MV->pushMatrix();
	{

		MV->translate(4, 0, 4);
		MV->scale(15, 1, 15);
		MV->rotate(-1 * M_PI / 2, { 1, 0, 0 });


		prog2->bind();
		glUniformMatrix4fv(prog2->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
		glUniformMatrix4fv(prog2->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
		glUniformMatrix4fv(prog2->getUniform("MVit"), 1, GL_FALSE, glm::value_ptr(transpose(inverse(MV->topMatrix()))));
		glUniform3f(prog2->getUniform("ka"), 0.2f, 0.2, 0.2);
		glUniform3f(prog2->getUniform("kd"), 0.8f, 0.0f, 0.0f);
		glUniform3f(prog2->getUniform("ks"), 1, 0.9, 0.8);
		glUniform1f(prog2->getUniform("s"), currMaterial.getShiny());
		plane->draw(prog2);
		prog2->unbind();
	

	}
	MV->popMatrix();
	
	// Draw Objects ***************************************************************************************************************
	for (int i = 0; i < objects.size(); i++) {

		currObject = objects[i];

		MV->pushMatrix();
		{
			MV->translate(currObject->getTranslation());
			MV->scale(currObject->getScale());

			prog2->bind();
			glUniformMatrix4fv(prog2->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
			glUniformMatrix4fv(prog2->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			glUniformMatrix4fv(prog2->getUniform("MVit"), 1, GL_FALSE, glm::value_ptr(transpose(inverse(MV->topMatrix()))));
			glUniform3f(prog2->getUniform("ka"), currMaterial.getAmbient()[0], currMaterial.getAmbient()[1], currMaterial.getAmbient()[2]);
			glUniform3f(prog2->getUniform("kd"), currObject->getColor()[0], currObject->getColor()[1], currObject->getColor()[2]);
			glUniform3f(prog2->getUniform("ks"), currMaterial.getSpecular()[0], currMaterial.getSpecular()[1], currMaterial.getSpecular()[2]);
			glUniform1f(prog2->getUniform("s"), currMaterial.getShiny());
			currObject->getShape()->draw(prog2);
			prog2->unbind();
		}
		MV->popMatrix();
	}



	// ***************************************************************************************************************************
	

	MV->popMatrix();
	P->popMatrix();

	GLSL::checkError(GET_FILE_LINE);
	
	if(OFFLINE) {
		saveImage("output.png", window);
		GLSL::checkError(GET_FILE_LINE);
		glfwSetWindowShouldClose(window, true);
	}
}

int main(int argc, char **argv)
{
	/*cout << minYCube << endl;
	cout << minYBunny << endl;*/
	if(argc < 2) {
		cout << "Usage: A3 RESOURCE_DIR" << endl;
		return 0;
	}
	RESOURCE_DIR = argv[1] + string("/");
	
	// Optional argument
	if(argc >= 3) {
		OFFLINE = atoi(argv[2]) != 0;
	}

	// Set error callback.
	glfwSetErrorCallback(error_callback);
	// Initialize the library.
	if(!glfwInit()) {
		return -1;
	}
	// Create a windowed mode window and its OpenGL context.
	window = glfwCreateWindow(640, 480, "YOUR NAME", NULL, NULL);
	if(!window) {
		glfwTerminate();
		return -1;
	}
	// Make the window's context current.
	glfwMakeContextCurrent(window);
	// Initialize GLEW.
	glewExperimental = true;
	if(glewInit() != GLEW_OK) {
		cerr << "Failed to initialize GLEW" << endl;
		return -1;
	}
	glGetError(); // A bug in glewInit() causes an error that we can safely ignore.
	cout << "OpenGL version: " << glGetString(GL_VERSION) << endl;
	cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
	GLSL::checkVersion();
	// Set vsync.
	glfwSwapInterval(1);
	// Set keyboard callback.
	glfwSetKeyCallback(window, key_callback);
	// Set char callback.
	glfwSetCharCallback(window, char_callback);
	// Set cursor position callback.
	glfwSetCursorPosCallback(window, cursor_position_callback);
	// Set mouse button callback.
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	// Set the window resize call back.
	glfwSetFramebufferSizeCallback(window, resize_callback);
	// Initialize scene.
	init();
	// Loop until the user closes the window.
	while(!glfwWindowShouldClose(window)) {
		// Render scene.
		render();
		// Swap front and back buffers.
		glfwSwapBuffers(window);
		// Poll for and process events.
		glfwPollEvents();
	}
	// Quit program.
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
