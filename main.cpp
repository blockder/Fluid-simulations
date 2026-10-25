#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<iostream>
#include"Mesh.h"
#include"Fluid.h"
#include"BLM.h"
#include"Fluid3.h"
#include"SPH.h"

//window settings
GLFWwindow* window;
int screenWidth = 800;
int screenHeight = 800;
float aspectRatio = float(screenHeight) / screenWidth;
const bool fullScreen = false;
const int FPS = 60;

void update(float dt, SPH& water);
void draw(Shader& render2d, Shader& renderPartical, SPH& water);

int main()
{
	//initialize glfw
	glfwInit();
	//specify the version of opengl
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	//tel opengl we are using only core profile
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//create a glfwWindow with size and name
	window = glfwCreateWindow(screenWidth, screenHeight, "openGl", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "falied to create window" << std::endl;
		glfwTerminate();
		return -1;
	}
	if (fullScreen)
	{
		glfwMaximizeWindow(window);
		glfwGetWindowSize(window, &screenWidth, &screenHeight);
	}

	glfwMakeContextCurrent(window);
	gladLoadGL();

	//openGL settings
	glViewport(0, 0, screenWidth, screenHeight);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//create shader
	Shader render2d("Shaders/render2d.vert", "Shaders/render2d.frag");
	Shader renderPartical("Shaders/renderPartical.vert", "Shaders/renderPartical.frag");
	
	SPH water(500);

	//main loop
	float prevFrame = 0;

	while (!glfwWindowShouldClose(window))
	{
		float currentTime = glfwGetTime();
		if (currentTime - prevFrame > 1.0 / FPS)
		{
			prevFrame = currentTime;
			update(1.0 / FPS, water);
			draw(render2d, renderPartical, water);
		}
		glfwPollEvents();
	}
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}

void update(float dt, SPH& water)
{
	water.input(window, screenWidth, screenHeight);
	if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
	{
		std::cout << "s";
	}
	water.integrate(dt);
}

void draw(Shader& render2d, Shader& renderPartical, SPH& water)
{
	// Clean buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	water.draw(renderPartical, aspectRatio);

	// display what was drawn
	glfwSwapBuffers(window);
}