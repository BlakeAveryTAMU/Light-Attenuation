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
#include "MyShape.h"
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
shared_ptr<Program> surface_of_rev;
shared_ptr<Shape> bunny;
shared_ptr<Shape> teapot;
shared_ptr<Shape> plane;
shared_ptr<Shape> sun;
shared_ptr<Shape> frustum;
shared_ptr<MyShape> sphere;
shared_ptr<MyShape> vase;

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
Light currLight;
int lightIndex = 0;

glm::vec3 light_positions[10];
glm::vec3 light_diffuse_colors[10];

vector<Object*> objects;
Object* currObject;

map<string, GLuint> bufIDs;
int indCount;

bool keyToggles[256] = {false}; // only for English keyboards!

//Get a random vec3 with numbers between 1.0 and 10.0
glm::vec3 randomizePosition() { return glm::vec3((float)rand() / (float)RAND_MAX * 9, 0.2, -1.0 * (float)rand() / (float)RAND_MAX * 9); }

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
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
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
	prog2->addUniform("lightPositions");
	prog2->addUniform("lightDiffuseColors");
	prog2->addUniform("ka");
	prog2->addUniform("ke");
	prog2->addUniform("kd");
	prog2->addUniform("ks");
	prog2->addUniform("s");
	prog2->setVerbose(false);
	prog2->addUniform("MVit");
	programs.push_back(prog2);

	surface_of_rev = make_shared<Program>();
	surface_of_rev->setShaderNames(RESOURCE_DIR + "surface_vert.glsl", RESOURCE_DIR + "frag.glsl");
	surface_of_rev->setVerbose(true);
	surface_of_rev->init();
	surface_of_rev->addUniform("t");
	surface_of_rev->addUniform("MV");
	surface_of_rev->addUniform("MVit");
	surface_of_rev->addUniform("P");
	surface_of_rev->addAttribute("aPos");
	surface_of_rev->addUniform("ka");
	surface_of_rev->addUniform("ke");
	surface_of_rev->addUniform("kd");
	surface_of_rev->addUniform("ks");
	surface_of_rev->addUniform("s");
	surface_of_rev->addUniform("lightPositions");
	surface_of_rev->addUniform("lightDiffuseColors");
	surface_of_rev->setVerbose(false);


	srand((unsigned int)time(NULL));

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

	Light l0;
	l0.setPosition(randomizePosition());
	l0.setDiffuse({ 1.0f, 0.0f, 0.0f }); //red
	lights.push_back(l0);

	Light l1;
	l1.setPosition(randomizePosition());
	l1.setDiffuse({ 0.0f, 1.0f, 0.0f }); // green
	lights.push_back(l1);

	Light l2;
	l2.setPosition(randomizePosition());
	l2.setDiffuse({ 0.0f, 0.0f, 1.0f }); // blue
	lights.push_back(l2);

	Light l3;
	l3.setPosition(randomizePosition());
	l3.setDiffuse({ 0.0f, 1.0f, 1.0f }); // light blue
	lights.push_back(l3);

	Light l4;
	l4.setPosition(randomizePosition());
	l4.setDiffuse({ 1.0f, 0.0f, 1.0f }); // pink
	lights.push_back(l4);

	Light l5;
	l5.setPosition(randomizePosition());
	l5.setDiffuse({ 1.0f, 1.0f, 0.0f }); // yellow
	lights.push_back(l5);

	Light l6;
	l6.setPosition(randomizePosition());
	l6.setDiffuse({ 1.0f, 0.5f, 0.0f }); // orange
	lights.push_back(l6);


	Light l7;
	l7.setPosition(randomizePosition());
	l7.setDiffuse({ 0.5f, 0.0f, 1.0f }); // purple
	lights.push_back(l7);

	Light l8;
	l8.setPosition(randomizePosition());
	l8.setDiffuse({ 0.0f, 1.0f, 0.5f }); // cyan green
	lights.push_back(l8);

	Light l9;
	l9.setPosition(randomizePosition());
	l9.setDiffuse({ 0.5f, 1.0f, 0.0f }); // yellow-green
	lights.push_back(l9);

	for (int i = 0; i < 10; i++) {
		light_positions[i] = lights[i].getPosition();
		light_diffuse_colors[i] = lights[i].getDiffuse();
	}


	//set default program
	currProgram = programs[0];
	//set the default material
	currMaterial = materials[0];
	//set the default light
	currLight = lights[0];

	camera = make_shared<Camera>();
	camera->setInitDistance(3.0f); // Camera's initial Z translation

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

	sphere = make_shared<MyShape>();
	sphere->loadShape("sphere");
	sphere->init();

	vase = make_shared<MyShape>();
	vase->loadShape("vase");
	vase->init();

	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 10; j++) {

			Object* obj = new Object();
			float r = rand() / float(RAND_MAX);
			//if (r >= 0.75) { r = 0.74; }

			if (r < 0.25) {

				obj->setShapeType("bunny");
				obj->setShape(bunny); // bunny
				obj->setTranslation(glm::vec3(j, obj->getScale()[1] * -0.333099, -i));
				
			}
			else if (r >= 0.25 && r < 0.5) {

				obj->setShapeType("teapot");
				obj->setShape(teapot); // teapot
				obj->setTranslation(glm::vec3(j, 0, -i));
				
			}
			else if (r >= 0.5 && r < 0.75) {

				obj->setShapeType("sphere");
				obj->setShape(sphere);
				float r_smaller = obj->smaller_random_scale_factor;
				if (r_smaller > 0.2){
					r_smaller = 0.2;
				}
				else if (r_smaller < 0.1) {
					r_smaller = 0.1;
				}
				obj->setScale(glm::vec3(r_smaller, r_smaller, r_smaller));
				obj->setTranslation(glm::vec3(j,obj->getScale()[1] * 1, -i));
			}
			else {
				
				obj->setShapeType("vase");
				obj->setShape(vase);
				obj->setScale(glm::vec3(0.07, 0.07, 0.07));
				obj->setTranslation(glm::vec3(j, 0, -i));
			}


			objects.push_back(obj);
		}
	}
	currObject = objects[0];

	
	
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
	
	//Draw Lights --------------------------------------------------------------------------------------
	glm::vec3 temp_light_positions[10];
	for (int i = 0; i < 10; i++) {

		temp_light_positions[i] = glm::vec3(MV->topMatrix() * glm::vec4(light_positions[i], 1));

	}

	prog2->bind();
	glUniform3fv(prog2->getUniform("lightPositions"), 10, glm::value_ptr(temp_light_positions[0]));
	glUniform3fv(prog2->getUniform("lightDiffuseColors"), 10, glm::value_ptr(light_diffuse_colors[0]));
	glUniform3f(prog2->getUniform("kd"), 0, 0, 0);
	glUniform3f(prog2->getUniform("ks"), 0, 0, 0);
	glUniform1f(prog2->getUniform("s"), 10);

	for (int i = 0; i < 10; i++) {

		MV->pushMatrix();
		{
			MV->translate(light_positions[i]);
			MV->scale(0.05, 0.05, 0.05);
			glUniformMatrix4fv(prog2->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
			glUniformMatrix4fv(prog2->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			glUniformMatrix4fv(prog2->getUniform("MVit"), 1, GL_FALSE, glm::value_ptr(transpose(inverse(MV->topMatrix()))));
			glUniform3f(prog2->getUniform("ke"), light_diffuse_colors[i][0], light_diffuse_colors[i][1], light_diffuse_colors[i][2]);
			sun->draw(prog2);
			
		}
		MV->popMatrix();
	}
	prog2->unbind();
	// ---------------------------------------------------------------------------------------------------

	glm::mat4 S(1.0f);
	S[1][2] = 0.5f * cos(t * 2);

	//Draw Ground
	MV->pushMatrix();
	{

		MV->translate(4, 0, -4);
		MV->scale(15, 1, 15);
		MV->rotate(-1 * M_PI / 2, { 1, 0, 0 });


		prog2->bind();
		glUniformMatrix4fv(prog2->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
		glUniformMatrix4fv(prog2->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
		glUniformMatrix4fv(prog2->getUniform("MVit"), 1, GL_FALSE, glm::value_ptr(transpose(inverse(MV->topMatrix()))));
		glUniform3f(prog2->getUniform("ke"), 0.0f, 0.0, 0.0);
		glUniform3f(prog2->getUniform("kd"), 0.8f, 0.8f, 0.8f);
		glUniform3f(prog2->getUniform("ks"), 1, 0.9, 0.8);
		glUniform1f(prog2->getUniform("s"), 10);
		plane->draw(prog2);
		prog2->unbind();
	

	}
	MV->popMatrix();

	float bounce_up = 1.3 * (1.0 / 2.0 * sin((2 * (M_PI / 1.7) * (t / 2.0 + 0.9))) + 1.0 / 2.0);
	float squash_and_stretch = -1 * 0.5 * (1.0 / 2.0 * cos((4 * M_PI / 1.7) * (t / 2.0 + 0.9)) + 1.0 / 2.0) + 1;
	
	// Draw Objects ***************************************************************************************************************
	for (int i = 0; i < objects.size(); i++) {

		currObject = objects[i];

		//cout << "Shape type: " << currObject->getShapeType() << endl;

		MV->pushMatrix();
		{
			MV->translate(currObject->getTranslation());
			MV->scale(currObject->getScale());

			if (currObject->getShapeType() == "bunny") {
				MV->rotate(t, { 0, 1, 0 });
			}
			if (currObject->getShapeType() == "teapot") {

				//shear
				MV->multMatrix(S);
			}
			if (currObject->getShapeType() == "sphere") {


				MV->translate(0, bounce_up, 0);
				MV->scale(squash_and_stretch, 1, squash_and_stretch);
			}
			if (currObject->getShapeType() == "vase") {

				MV->rotate(M_PI / 2, { 0, 0, 1 });
			}
			
			prog2->bind();
			glUniformMatrix4fv(prog2->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
			glUniformMatrix4fv(prog2->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			glUniformMatrix4fv(prog2->getUniform("MVit"), 1, GL_FALSE, glm::value_ptr(transpose(inverse(MV->topMatrix()))));
			glUniform3f(prog2->getUniform("ke"), 0, 0, 0);
			glUniform3f(prog2->getUniform("kd"), currObject->getDiffuse()[0], currObject->getDiffuse()[1], currObject->getDiffuse()[2]);
			glUniform3f(prog2->getUniform("ks"), 1, 1, 1);
			glUniform1f(prog2->getUniform("s"), 10);

			if (currObject->getShapeType() == "sphere") {
				currObject->getMyShape()->draw(prog2, "sphere", t);
			}
			else if (currObject->getShapeType() == "vase") {

				surface_of_rev->bind();
				glUniform3fv(surface_of_rev->getUniform("lightPositions"), 10, glm::value_ptr(temp_light_positions[0]));
				glUniform3fv(surface_of_rev->getUniform("lightDiffuseColors"), 10, glm::value_ptr(light_diffuse_colors[0]));
				glUniformMatrix4fv(surface_of_rev->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
				glUniformMatrix4fv(surface_of_rev->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
				glUniformMatrix4fv(surface_of_rev->getUniform("MVit"), 1, GL_FALSE, glm::value_ptr(transpose(inverse(MV->topMatrix()))));
				glUniform3f(surface_of_rev->getUniform("ke"), 0, 0, 0);
				glUniform3f(surface_of_rev->getUniform("kd"), currObject->getDiffuse()[0], currObject->getDiffuse()[1], currObject->getDiffuse()[2]);
				glUniform3f(surface_of_rev->getUniform("ks"), 1, 1, 1);
				glUniform1f(surface_of_rev->getUniform("s"), 10);
				currObject->getMyShape()->draw(surface_of_rev, "vase", t);
				surface_of_rev->unbind();
			}
			else {
				currObject->getShape()->draw(prog2);
			}

			GLSL::checkError(GET_FILE_LINE);

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
	window = glfwCreateWindow(640, 480, "Blake Avery", NULL, NULL);
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
