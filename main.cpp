//imgui library
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
//#include "imgui_impl_opengl3_loader.h"

//Glad and glfw
#include<glad/glad.h>
#include <GLFW/glfw3.h>

//Glm
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Sphere.h"
#include "Shader.h"
#include "Camera.h"

//stb image library
#include <stb_image.h>

#include <iostream>
#include <vector>
#include <map>
#include <ctime>
#include <filesystem>
#include <string>
#include <cstdlib>
#include <wtypes.h>

#define _USE_MATH_DEFINES
#include <math.h>

#define TAU (M_PI * 2.0)

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
unsigned int loadTexture(char const* path);
unsigned int loadCubemap(std::vector<std::string> faces);
void RenderPlanetInfoPopup(bool* showPopup, const char* planetName, const char* planetInfo,
	const char* mass, const char* diameter, const char* numSatellites,
	const char* rotationDir, const char* rotationTime, const char* revolutionTime);
void GetDesktopResolution(float& horizontal, float& vertical)
{
	//Structure in windows that represents a rectangle, usually used for defining the dimensions of a window or an area. 
	RECT desktop;
	// Get a handle to the desktop window
	const HWND hDesktop = GetDesktopWindow();
	// Get the size of screen to the variable desktop
	GetWindowRect(hDesktop, &desktop);
	// The top left corner will have coordinates (0,0)
	// and the bottom right corner will have coordinates
	// (horizontal, vertical)
	horizontal = desktop.right;
	vertical = desktop.bottom;

}
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;
//we are using right handed coordinate system thus,it places the camera at the origin of the x-y plane
// (the center of the screen) and then moves it 3 units along the positive z-axis
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
bool onRotate = false;
//bool onFreeCam = true;
bool SkyBoxExtra = false;
float SCREEN_WIDTH = 800, SCREEN_HEIGHT = 600;

glm::vec3 point = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 PlanetPos = glm::vec3(0.0f, 0.0f, 0.0f);
GLfloat lastX = (GLfloat)(SCREEN_WIDTH / 2.0);
GLfloat lastY = (GLfloat)(SCREEN_HEIGHT / 2.0);
float PlanetSpeed = .1f;
int PlanetView = 0;

bool keys[1024];
GLfloat SceneRotateY = 0.0f;
GLfloat SceneRotateX = 0.0f;
bool onPlanet = false;

/* PLANETS INFO POPUPS*/
bool showMercuryPopUp = false;
bool showVenusPopUp = false;
bool showEarthPopUp = false;
bool showMarsPopUp = false;
bool showJupiterPopUp = false;
bool showSaturnPopUp = false;
bool showNeptunePopUp = false;
bool showUranusPopUp = false;
//bool showSunPopUp = false;
/* PLANETS INFO POPUPS*/

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	/*if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
	{
		//camera.Position = PlanetPos;
		onPlanet = true;
	}*/

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			keys[key] = true;
		else if (action == GLFW_RELEASE)
			keys[key] = false;
	}
}

bool firstMouse = true;
GLfloat xoff = 0.0f, yoff = 0.0f;

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = (GLfloat)xpos;
		lastY = (GLfloat)ypos;
		firstMouse = false;
	}

	GLfloat xoffset = (GLfloat)(xpos - lastX);
	GLfloat yoffset = (GLfloat)(lastY - ypos);
	xoff = xoffset;
	yoff = yoff;										//yoffset maybe?

	lastX = (GLfloat)xpos;
	lastY = (GLfloat)ypos;
	if (onRotate)
	{
		SceneRotateY += yoffset * 0.1f;
		SceneRotateX += xoffset * 0.1f;
	}
	camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (yoffset == 1)
		camera.ProcessKeyboard(SCROLL_FORWARD, deltaTime);
	else
	{
		camera.ProcessKeyboard(SCROLL_BACKWARD, deltaTime);
	}
}

int main() {
	GetDesktopResolution(SCREEN_WIDTH, SCREEN_HEIGHT); // get resolution for create window
	camera.LookAtPos = point;

	/* GLFW INIT */
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);
	/* GLFW INIT */

	/* GLFW WINDOW CREATION */
	GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "LearnOpenGL", glfwGetPrimaryMonitor(), NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetScrollCallback(window, scroll_callback);
	//glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	/* GLFW WINDOW CREATION */

	/* LOAD GLAD */
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	/* LOAD GLAD */


	/* IMGUI SETUP */
	//Imgui initialisation
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	//variable for inputs and outputs
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	//Choose style
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");
	/* IMGUI SETUP */

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	/* SHADERS */
	Shader SimpleShader("simpleVs.vs", "simpleFs.frag");
	Shader SkyboxShader("skybox.vs", "skybox.frag");
	Shader texShader("simpleVs.vs", "texFs.frag");
	/* SHADERS */

	float cube[] = {
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
	 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
	-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	 0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
	 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
	};
	float skyboxVertices[] = {
		// positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};

	/* SKYBOX GENERATION */
	unsigned int skyboxVAO, skyboxVBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	/* SKYBOX GENERATION */

	/* VERTEX GENERATION FOR ORBITS */
	std::vector<float> orbVert;
	GLfloat xx;
	GLfloat zz;
	float angl;
	for (int i = 0; i < 2000; i++)
	{
		angl = (float)(M_PI / 2 - i * (M_PI / 1000));
		xx = sin(angl) * 100.0f;
		zz = cos(angl) * 100.0f;
		orbVert.push_back(xx);
		orbVert.push_back(0.0f);
		orbVert.push_back(zz);

	}
	/* VERTEX GENERATION FOR ORBITS */

	/* VAO-VBO for ORBITS*/
	GLuint VBO_t, VAO_t;
	glGenVertexArrays(1, &VAO_t);
	glGenBuffers(1, &VBO_t);
	glBindVertexArray(VAO_t);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_t);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * orbVert.size(), orbVert.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	/* VAO-VBO for ORBITS*/

	/* LOAD TEXTURES */
	unsigned int texture_earth = loadTexture("resources/planets/earth2k.jpg");
	unsigned int t_sun = loadTexture("resources/planets/2k_sun.jpg");
	unsigned int texture_moon = loadTexture("resources/planets/2k_moon.jpg");
	unsigned int texture_mercury = loadTexture("resources/planets/2k_mercury.jpg");
	unsigned int texture_venus = loadTexture("resources/planets/2k_mercury.jpg");
	unsigned int texture_mars = loadTexture("resources/planets/2k_mars.jpg");
	unsigned int texture_jupiter = loadTexture("resources/planets/2k_jupiter.jpg");
	unsigned int texture_saturn = loadTexture("resources/planets/2k_saturn.jpg");
	unsigned int texture_uranus = loadTexture("resources/planets/2k_uranus.jpg");
	unsigned int texture_neptune = loadTexture("resources/planets/2k_neptune.jpg");
	unsigned int texture_saturn_ring = loadTexture("resources/planets/r.jpg");
	unsigned int texture_earth_clouds = loadTexture("resources/planets/2k_earth_clouds.jpg");
	/* LOAD TEXTURES */

	/* SPHERE GENERATION */
	Sphere Sun(100.0f, 36 * 6, 18 * 6);
	Sphere Mercury(10.0f, 36, 18);
	Sphere Venus(12.0f, 36, 18);
	Sphere Earth(11.8f, 36, 18);
	Sphere Mars(8.0f, 36, 18);
	Sphere Jupiter(40.0f, 36 * 4, 18 * 4);
	Sphere Saturn(37.0f, 36 * 3, 18 * 3);
	Sphere Uranus(30.0f, 36 * 2, 18 * 2);
	Sphere Neptune(30.0f, 36 * 2, 18 * 2);
	Sphere Moon(5.5f, 36, 18);
	/* SPHERE GENERATION */

	std::vector<std::string> faces
	{
		"resources/skybox/starfield/starfield_rt.tga",
		"resources/skybox/starfield/starfield_lf.tga",
		"resources/skybox/starfield/starfield_up.tga",
		"resources/skybox/starfield/starfield_dn.tga",
		"resources/skybox/starfield/starfield_ft.tga",
		"resources/skybox/starfield/starfield_bk.tga",
	};
	std::vector<std::string> faces_extra
	{
		"resources/skybox/blue/bkg1_right.png",
		"resources/skybox/blue/bkg1_left.png",
		"resources/skybox/blue/bkg1_top.png",
		"resources/skybox/blue/bkg1_bot.png",
		"resources/skybox/blue/bkg1_front.png",
		"resources/skybox/blue/bkg1_back.png",
	};

	unsigned int cubemapTexture = loadCubemap(faces);
	unsigned int cubemapTextureExtra = loadCubemap(faces_extra);
	GLfloat camX = 10.0f;
	GLfloat camZ = 10.0f;

	camera.Position = glm::vec3(0.0f, 250.0f, -450.0f);
	camera.Yaw = 90.0f;
	camera.Pitch = -40.0f;
	camera.ProcessMouseMovement(xoff, yoff);
	//camera.FreeCam = false;
	//onFreeCam = true;
	glm::mat4 view;
	glm::vec3 PlanetsPositions[9];


	while (!glfwWindowShouldClose(window))
	{
		GLfloat currentFrame = (GLfloat)glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;


		//view = camera.GetViewMatrix();


		//simulate the effects of gravity
		/* ZOOM CONTROL */
		if (camera.Position.y > 200.0f && camera.Position.y < 400.0f)
			camera.MovementSpeed = 400.0f;
		else if (camera.Position.y > 125.0f && camera.Position.y < 200.0f)
			camera.MovementSpeed = 300.0f;
		else if (camera.Position.y > 70.0f && camera.Position.y < 125.0f)
			camera.MovementSpeed = 200.0f;
		else if (camera.Position.y > 50.0f)
			camera.MovementSpeed = 100.0f;
		/* ZOOM CONTROL */

		processInput(window);				//input

		// render
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Let ImGui know we are working on a new frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();


		glm::mat4 model = glm::mat4(1.0f);

		double viewX;
		double viewZ;
		glm::vec3 viewPos;

		SimpleShader.Use();
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 10000.0f);
		SimpleShader.setMat4("model", model);
		SimpleShader.setMat4("view", view);
		SimpleShader.setMat4("projection", projection);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, t_sun);

		/* SUN */
		glm::mat4 model_sun;
		model_sun = glm::rotate(model_sun, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.f));
		model_sun = glm::rotate(model_sun, (GLfloat)glfwGetTime() * glm::radians(23.5f) * 0.25f, glm::vec3(0.0f, 0.0f, 1.f));
		model_sun = glm::translate(model_sun, point);
		SimpleShader.setMat4("model", model_sun);
		Sun.Draw();
		/* SUN */

		/* MERCURY */
		glm::mat4 model_mercury;
		double xx = sin(glfwGetTime() * PlanetSpeed) * 100.0f * 2.0f * 1.3f;
		double zz = cos(glfwGetTime() * PlanetSpeed) * 100.0f * 2.0f * 1.3f;
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_mercury);
		model_mercury = glm::translate(model_mercury, point);
		model_mercury = glm::translate(model_mercury, glm::vec3(xx, 0.0f, zz));
		PlanetsPositions[0] = glm::vec3(xx, 0.0f, zz);
		//additional tilting or orientation change due to the X-axis rotation i.e. axial tilt
		model_mercury = glm::rotate(model_mercury, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.f));
		model_mercury = glm::rotate(model_mercury, (GLfloat)glfwGetTime() * glm::radians(90.0f) * 0.05f, glm::vec3(0.0f, 0.0f, 1.f));
		SimpleShader.setMat4("model", model_mercury);
		Mercury.Draw();
		/* MERCURY */

		/* VENUS */
		glm::mat4 model_venus;
		xx = sin(glfwGetTime() * PlanetSpeed * 0.75f) * 100.0f * 3.0f * 1.3f;
		zz = cos(glfwGetTime() * PlanetSpeed * 0.75f) * 100.0f * 3.0f * 1.3f;
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_venus);
		model_venus = glm::translate(model_venus, point);
		model_venus = glm::translate(model_venus, glm::vec3(xx, 0.0f, zz));
		PlanetsPositions[1] = glm::vec3(xx, 0.0f, zz);
		model_venus = glm::rotate(model_venus, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.f));
		model_venus = glm::rotate(model_venus, glm::radians(-132.5f), glm::vec3(0.0f, 1.0f, 0.f));
		model_venus = glm::rotate(model_venus, (GLfloat)glfwGetTime() * glm::radians(-132.5f) * 0.012f, glm::vec3(0.0f, 0.0f, 1.f));
		SimpleShader.setMat4("model", model_venus);
		Venus.Draw();
		/* VENUS */

		/* EARTH */
		glm::mat4 model_earth;
		xx = sin(glfwGetTime() * PlanetSpeed * 0.55f) * 100.0f * 4.0f * 1.3f;
		zz = cos(glfwGetTime() * PlanetSpeed * 0.55f) * 100.0f * 4.0f * 1.3f;
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_earth);
		model_earth = glm::translate(model_earth, point);
		model_earth = glm::translate(model_earth, glm::vec3(xx, 0.0f, zz));
		glm::vec3 EarthPoint = glm::vec3(xx, 0.0f, zz);
		PlanetsPositions[2] = glm::vec3(xx, 0.0f, zz);
		model_earth = glm::rotate(model_earth, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.f));
		model_earth = glm::rotate(model_earth, glm::radians(-33.25f), glm::vec3(0.0f, 1.0f, 0.f));
		model_earth = glm::rotate(model_earth, (GLfloat)glfwGetTime() * glm::radians(33.25f) * 2.0f, glm::vec3(0.0f, 0.0f, 1.f));
		camera.LookAtPos = glm::vec3(model_earth[3][0], model_earth[3][1], model_earth[3][2]);
		SimpleShader.setMat4("model", model_earth);
		Earth.Draw();

		/* EARTH */

		/* MOON */
		glm::mat4 model_moon;
		xx = sin(glfwGetTime() * PlanetSpeed * 67.55f) * 100.0f * 0.5f * 1.3f;
		zz = cos(glfwGetTime() * PlanetSpeed * 67.55f) * 100.0f * 0.5f * 1.3f;
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_moon);
		model_moon = glm::translate(model_moon, EarthPoint);
		model_moon = glm::translate(model_moon, glm::vec3(xx, 0.0f, zz));
		model_moon = glm::rotate(model_moon, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.f));
		model_moon = glm::rotate(model_moon, glm::radians(-32.4f), glm::vec3(0.0f, 1.0f, 0.f));
		model_moon = glm::rotate(model_moon, (GLfloat)glfwGetTime() * glm::radians(32.4f) * 3.1f, glm::vec3(0.0f, 0.0f, 1.f));
		SimpleShader.setMat4("model", model_moon);
		Moon.Draw();
		/* MOON */


		/* MARS */
		glm::mat4 model_mars;
		xx = sin(glfwGetTime() * PlanetSpeed * 0.35f) * 100.0f * 5.0f * 1.3f;
		zz = cos(glfwGetTime() * PlanetSpeed * 0.35f) * 100.0f * 5.0f * 1.3f;
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_mars);
		model_mars = glm::translate(model_mars, point);
		model_mars = glm::translate(model_mars, glm::vec3(xx, 0.0f, zz));
		PlanetsPositions[3] = glm::vec3(xx, 0.0f, zz);
		model_mars = glm::rotate(model_mars, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.f));
		model_mars = glm::rotate(model_mars, glm::radians(-32.4f), glm::vec3(0.0f, 1.0f, 0.f));
		model_mars = glm::rotate(model_mars, (GLfloat)glfwGetTime() * glm::radians(32.4f) * 2.1f, glm::vec3(0.0f, 0.0f, 1.f));
		SimpleShader.setMat4("model", model_mars);
		Mars.Draw();
		/* MARS */

		/* JUPITER */
		glm::mat4 model_jupiter;
		xx = sin(glfwGetTime() * PlanetSpeed * 0.2f) * 100.0f * 6.0f * 1.3f;
		zz = cos(glfwGetTime() * PlanetSpeed * 0.2f) * 100.0f * 6.0f * 1.3f;
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_jupiter);
		model_jupiter = glm::translate(model_jupiter, point);
		model_jupiter = glm::translate(model_jupiter, glm::vec3(xx, 0.0f, zz));
		PlanetsPositions[4] = glm::vec3(xx, 0.0f, zz);
		model_jupiter = glm::rotate(model_jupiter, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.f));
		model_jupiter = glm::rotate(model_jupiter, glm::radians(-23.5f), glm::vec3(0.0f, 1.0f, 0.f));
		model_jupiter = glm::rotate(model_jupiter, (GLfloat)glfwGetTime() * glm::radians(23.5f) * 4.5f, glm::vec3(0.0f, 0.0f, 1.f));
		SimpleShader.setMat4("model", model_jupiter);
		Jupiter.Draw();
		/* JUPITER */

		/* SATURN */
		glm::mat4 model_saturn;
		xx = sin(glfwGetTime() * PlanetSpeed * 0.15f) * 100.0f * 7.0f * 1.3f;
		zz = cos(glfwGetTime() * PlanetSpeed * 0.15f) * 100.0f * 7.0f * 1.3f;
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_saturn);
		model_saturn = glm::translate(model_saturn, point);
		model_saturn = glm::translate(model_saturn, glm::vec3(xx, 0.0f, zz));
		glm::vec3 SatrunPoint = glm::vec3(xx, 0.0f, zz);
		PlanetsPositions[5] = glm::vec3(xx, 0.0f, zz);
		model_saturn = glm::rotate(model_saturn, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.f));
		model_saturn = glm::rotate(model_saturn, glm::radians(-34.7f), glm::vec3(0.0f, 1.0f, 0.f));
		model_saturn = glm::rotate(model_saturn, (GLfloat)glfwGetTime() * glm::radians(34.7f) * 4.48f, glm::vec3(0.0f, 0.0f, 1.f));
		SimpleShader.setMat4("model", model_saturn);
		Saturn.Draw();
		/* SATURN */

		/* URANUS */
		glm::mat4 model_uranus;
		xx = sin(glfwGetTime() * PlanetSpeed * 0.1f) * 100.0f * 8.0f * 1.3f;
		zz = cos(glfwGetTime() * PlanetSpeed * 0.1f) * 100.0f * 8.0f * 1.3f;
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_uranus);
		model_uranus = glm::translate(model_uranus, point);
		model_uranus = glm::translate(model_uranus, glm::vec3(xx, 0.0f, zz));
		PlanetsPositions[6] = glm::vec3(xx, 0.0f, zz);
		model_uranus = glm::rotate(model_uranus, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.f));
		model_uranus = glm::rotate(model_uranus, glm::radians(-99.0f), glm::vec3(0.0f, 1.0f, 0.f));
		model_uranus = glm::rotate(model_uranus, (GLfloat)glfwGetTime() * glm::radians(-99.0f) * 4.5f, glm::vec3(0.0f, 0.0f, 1.f));
		SimpleShader.setMat4("model", model_uranus);
		Uranus.Draw();
		/* URANUS */

		/* NEPTUNE */
		glm::mat4 model_neptune;
		xx = sin(glfwGetTime() * PlanetSpeed * 0.08f) * 100.0f * 9.0f * 1.3f;
		zz = cos(glfwGetTime() * PlanetSpeed * 0.08f) * 100.0f * 9.0f * 1.3f;
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_neptune);

		model_neptune = glm::translate(model_neptune, point);
		model_neptune = glm::translate(model_neptune, glm::vec3(xx, 0.0f, zz));
		PlanetsPositions[7] = glm::vec3(xx, 0.0f, zz);
		model_neptune = glm::rotate(model_neptune, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.f));
		model_neptune = glm::rotate(model_neptune, glm::radians(-30.2f), glm::vec3(0.0f, 1.0f, 0.f));
		model_neptune = glm::rotate(model_neptune, (GLfloat)glfwGetTime() * glm::radians(30.2f) * 4.0f, glm::vec3(0.0f, 0.0f, 1.f));

		SimpleShader.setMat4("model", model_neptune);
		Neptune.Draw();
		/* NEPTUNE */


		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_venus);

		/* ORBITS */
		glBindVertexArray(VAO_t);
		glLineWidth(1.0f);
		glm::mat4 modelorb;
		for (float i = 2; i < 10; i++)
		{
			modelorb = glm::mat4(1);
			modelorb = glm::translate(modelorb, point);
			modelorb = glm::scale(modelorb, glm::vec3(i * 1.3f, i * 1.3f, i * 1.3f));
			SimpleShader.setMat4("model", modelorb);
			glDrawArrays(GL_LINE_LOOP, 0, (GLsizei)orbVert.size() / 3);

		}
		modelorb = glm::mat4(1);
		modelorb = glm::translate(modelorb, EarthPoint);
		modelorb = glm::scale(modelorb, glm::vec3(0.5f * 1.3f, 0.5f * 1.3f, 0.5f * 1.3f));
		SimpleShader.setMat4("model", modelorb);
		glDrawArrays(GL_LINE_LOOP, 0, (GLsizei)orbVert.size() / 3);
		/* ORBITS */

		/* SATURN RINGS */
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_saturn_ring);
		glLineWidth(2.0f);
		GLfloat rr = 0.55f;
		for (int i = 0; i < 25; i++)
		{
			modelorb = glm::mat4(1);
			modelorb = glm::translate(modelorb, SatrunPoint);
			modelorb = glm::rotate(modelorb, glm::radians(30.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			modelorb = glm::scale(modelorb, glm::vec3(rr, rr, rr));
			SimpleShader.setMat4("model", modelorb);
			glDrawArrays(GL_LINE_LOOP, 0, (GLsizei)orbVert.size() / 3);
			if (i == 15)
				rr += 0.030f;
			else
				rr += 0.01f;
		}
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_venus);
		glBindVertexArray(0);
		/* SATURN RINGS */


		/* DRAW SKYBOX */
		glDepthFunc(GL_LEQUAL);
		SkyboxShader.Use();
		view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
		SkyboxShader.setMat4("view", view);
		SkyboxShader.setMat4("projection", projection);
		// skybox cube
		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);
		if (SkyBoxExtra)
			glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTextureExtra);
		else
			glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		/* DRAW IMGUI WINDOW */
		/*ImGui::Begin("My name is window, ImGui window");
		ImGui::Text("Hello there adventurer!");
		ImGui::End();
		/* DRAW IMGUI WINDOW */

		glBindVertexArray(0);
		glDepthFunc(GL_LESS);
		/* DRAW SKYBOX */

		/* PLANET TRACKING + SHOW INFO OF PLANET */
		switch (PlanetView)
		{
		case 1:
			viewX = sin(glfwGetTime() * PlanetSpeed) * 100.0f * 3.5f * 1.3f;
			viewZ = cos(glfwGetTime() * PlanetSpeed) * 100.0f * 3.5f * 1.3f;
			viewPos = glm::vec3(viewX, 50.0f, viewZ);
			view = glm::lookAt(viewPos, PlanetsPositions[0], glm::vec3(0.0f, 1.0f, 0.0f));
			if (showMercuryPopUp) {
				RenderPlanetInfoPopup(&showMercuryPopUp, "Mercury", "Mercury is the smallest planet in our Solar System.",
					"0.055 Earth masses", "4,880 km", "0", "CounterClockwise", "59 days", "88 days");
			}
			break;

		case 2:
			viewX = sin(glfwGetTime() * PlanetSpeed * 0.75f) * 100.0f * 4.5f * 1.2f;
			viewZ = cos(glfwGetTime() * PlanetSpeed * 0.75f) * 100.0f * 4.5f * 1.2f;
			viewPos = glm::vec3(viewX, 50.0f, viewZ);
			view = glm::lookAt(viewPos, PlanetsPositions[1], glm::vec3(0.0f, 1.0f, 0.0f));
			if (showVenusPopUp) {
				RenderPlanetInfoPopup(&showVenusPopUp, "Venus", "Venus is often called Earth's twin because of their similar sizes, masses, and rocky compositions.",
					"0.82 Earth masses", "12,104 km", "0", "Clockwise", "243 days", "225 days");
			}
			break;

		case 3:
			viewX = sin(glfwGetTime() * PlanetSpeed * 0.55f) * 100.0f * 5.5f * 1.2f;
			viewZ = cos(glfwGetTime() * PlanetSpeed * 0.55f) * 100.0f * 5.5f * 1.2f;
			viewPos = glm::vec3(viewX, 50.0f, viewZ);
			view = glm::lookAt(viewPos, PlanetsPositions[2], glm::vec3(0.0f, 1.0f, 0.0f));
			if (showEarthPopUp) {
				RenderPlanetInfoPopup(&showEarthPopUp, "Earth", "Earth is the third planet from the Sun and the only known planet to support life.",
					"1 Earth mass", "12,742 km", "1", "Counterclockwise", "24 hours", "365 days");
			}
			break;

		case 4:
			viewX = sin(glfwGetTime() * PlanetSpeed * 0.35f) * 100.0f * 6.0f * 1.2f;
			viewZ = cos(glfwGetTime() * PlanetSpeed * 0.35f) * 100.0f * 6.0f * 1.2f;
			viewPos = glm::vec3(viewX, 20.0f, viewZ);
			view = glm::lookAt(viewPos, PlanetsPositions[3], glm::vec3(0.0f, 1.0f, 0.0f));
			if (showMarsPopUp) {
				RenderPlanetInfoPopup(&showMarsPopUp, "Mars", "Mars is often called the \"Red Planet\" due to its reddish appearance.",
					"0.11 Earth masses", "6,779 km", "2", "CounterClockwise", "25 hours", "687 days");
			}
			break;

		case 5:
			viewX = sin(glfwGetTime() * PlanetSpeed * 0.2f) * 100.0f * 7.5f * 1.3f;
			viewZ = cos(glfwGetTime() * PlanetSpeed * 0.2f) * 100.0f * 7.5f * 1.3f;
			viewPos = glm::vec3(viewX, 50.0f, viewZ);
			view = glm::lookAt(viewPos, PlanetsPositions[4], glm::vec3(0.0f, 1.0f, 0.0f));
			if (showJupiterPopUp) {
				RenderPlanetInfoPopup(&showJupiterPopUp, "Jupiter", "Jupiter is the largest planet in our Solar System.",
					"317.8 Earth masses", "139,820 km", "79", "Counterclockwise", "10 hours", "12 years");
			}
			break;

		case 6:
			viewX = sin(glfwGetTime() * PlanetSpeed * 0.15f) * 100.0f * 8.5f * 1.3f;
			viewZ = cos(glfwGetTime() * PlanetSpeed * 0.15f) * 100.0f * 8.5f * 1.3f;
			viewPos = glm::vec3(viewX, 50.0f, viewZ);
			view = glm::lookAt(viewPos, PlanetsPositions[5], glm::vec3(0.0f, 1.0f, 0.0f));
			if (showSaturnPopUp) {
				RenderPlanetInfoPopup(&showSaturnPopUp, "Saturn", "Saturn is known for its stunning ring system.",
					"95.2 Earth masses", "116,460 km", "83", "Counterclockwise", "10.5 hours", "29.5 years");
			}
			break;

		case 7:
			viewX = sin(glfwGetTime() * PlanetSpeed * 0.1f) * 100.0f * 9.5f * 1.3f;
			viewZ = cos(glfwGetTime() * PlanetSpeed * 0.1f) * 100.0f * 9.5f * 1.3f;
			viewPos = glm::vec3(viewX, 50.0f, viewZ);
			view = glm::lookAt(viewPos, PlanetsPositions[6], glm::vec3(0.0f, 1.0f, 0.0f));
			if (showUranusPopUp) {
				RenderPlanetInfoPopup(&showUranusPopUp, "Uranus", "Uranus is the seventh planet from the Sun.",
					"14.5 Earth masses", "50,724 km", "27", "Clockwise", "17 hours", "84 years");
			}

			break;

		case 8:
			viewX = sin(glfwGetTime() * PlanetSpeed * 0.08f) * 100.0f * 10.5f * 1.3f;
			viewZ = cos(glfwGetTime() * PlanetSpeed * 0.08f) * 100.0f * 10.5f * 1.3f;
			viewPos = glm::vec3(viewX, 50.0f, viewZ);
			view = glm::lookAt(viewPos, PlanetsPositions[7], glm::vec3(0.0f, 1.0f, 0.0f));
			if (showNeptunePopUp) {
				RenderPlanetInfoPopup(&showNeptunePopUp, "Neptune", "Neptune is the eighth and farthest known planet from the Sun.",
					"17.2 Earth masses", "49,530 km", "14", "Counterclockwise", "16 hours", "165 years");
			}
			break;

		case 0:
			view = camera.GetViewMatrix();
			break;
		}

		//Render UI elements
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		/* PLANET TRACKING + SHOW INFO OF PLANET */

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
			// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	//End the ImGui processes
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glDeleteVertexArrays(1, &VAO_t);
	glDeleteBuffers(1, &VBO_t);
	glfwTerminate();
	return 0;

}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);

	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		SkyBoxExtra = true;
	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
		SkyBoxExtra = false;
	if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); // Enable mouse cursor
	if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Enable mouse cursor

	if (glfwGetKey(window, GLFW_KEY_SPACE) || (glfwGetKey(window, GLFW_KEY_0)) == GLFW_PRESS)
	{
		PlanetView = 0;
		camera.Position = glm::vec3(0.0f, 250.0f, -450.0f);
		camera.Yaw = 90.0f;
		camera.Pitch = -40.0f;
		camera.GetViewMatrix();
		camera.ProcessMouseMovement(xoff, yoff);

		showMercuryPopUp = false;
		showVenusPopUp = false;
		showEarthPopUp = false;
		showMarsPopUp = false;
		showJupiterPopUp = false;
		showSaturnPopUp = false;
		showNeptunePopUp = false;
		showUranusPopUp = false;
	}
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
	{
		PlanetView = 1;
		showMercuryPopUp = true;
		showVenusPopUp = false;
		showEarthPopUp = false;
		showMarsPopUp = false;
		showJupiterPopUp = false;
		showSaturnPopUp = false;
		showNeptunePopUp = false;
		showUranusPopUp = false;
	}
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
	{
		PlanetView = 2;
		showVenusPopUp = true;
		showMercuryPopUp = false;
		showEarthPopUp = false;
		showMarsPopUp = false;
		showJupiterPopUp = false;
		showSaturnPopUp = false;
		showNeptunePopUp = false;
		showUranusPopUp = false;
	}
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
	{
		PlanetView = 3;
		showEarthPopUp = true;
		showMercuryPopUp = false;
		showVenusPopUp = false;
		showMarsPopUp = false;
		showJupiterPopUp = false;
		showSaturnPopUp = false;
		showNeptunePopUp = false;
		showUranusPopUp = false;
	}
	if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
	{
		PlanetView = 4;
		showMarsPopUp = true;
		showMercuryPopUp = false;
		showVenusPopUp = false;
		showEarthPopUp = false;
		showJupiterPopUp = false;
		showSaturnPopUp = false;
		showNeptunePopUp = false;
		showUranusPopUp = false;
	}
	if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS)
	{
		PlanetView = 5;
		showJupiterPopUp = true;
		showMercuryPopUp = false;
		showVenusPopUp = false;
		showEarthPopUp = false;
		showMarsPopUp = false;
		showSaturnPopUp = false;
		showNeptunePopUp = false;
		showUranusPopUp = false;
	}
	if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS)
	{
		PlanetView = 6;
		showSaturnPopUp = true;
		showMercuryPopUp = false;
		showVenusPopUp = false;
		showEarthPopUp = false;
		showMarsPopUp = false;
		showJupiterPopUp = false;
		showNeptunePopUp = false;
		showUranusPopUp = false;
	}
	if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS)
	{
		PlanetView = 7;
		showNeptunePopUp = false;
		showMercuryPopUp = false;
		showVenusPopUp = false;
		showEarthPopUp = false;
		showMarsPopUp = false;
		showJupiterPopUp = false;
		showSaturnPopUp = false;
		showUranusPopUp = true;
	}
	if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS)
	{
		PlanetView = 8;
		showUranusPopUp = false;
		showMercuryPopUp = false;
		showVenusPopUp = false;
		showEarthPopUp = false;
		showMarsPopUp = false;
		showJupiterPopUp = false;
		showSaturnPopUp = false;
		showNeptunePopUp = true;
	}
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}
unsigned int loadCubemap(std::vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;

	for (unsigned int i = 0; i < faces.size(); i++)
	{

		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);

		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
			);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}
unsigned int loadTexture(char const* path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}
void RenderPlanetInfoPopup(bool* showPopup, const char* planetName, const char* planetInfo,
	const char* mass, const char* diameter, const char* numSatellites,
	const char* rotationDir, const char* rotationTime, const char* revolutionTime)
{
	if (*showPopup) {
		ImGui::OpenPopup("Planet Info");
		ImGui::SetNextWindowPos(ImVec2(10, ImGui::GetIO().DisplaySize.y / 2 - 150), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_Always);

		if (ImGui::Begin("Planet Info", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
			ImGui::Text("%s", planetName);
			ImGui::Separator();
			ImGui::TextWrapped("%s", planetInfo);

			ImGui::Separator();
			ImGui::Text("Planet Details:");
			ImGui::Text("Mass: %s", mass);
			ImGui::Text("Diameter: %s", diameter);
			ImGui::Text("Number of Satellites: %s", numSatellites);
			ImGui::Text("Rotation Direction: %s", rotationDir);
			ImGui::Text("Rotation Time: %s", rotationTime);
			ImGui::Text("Revolution Time: %s", revolutionTime);

			if (ImGui::Button("Close")) {
				*showPopup = false;
				ImGui::CloseCurrentPopup();
			}
			ImGui::End();

		}
	}
}
