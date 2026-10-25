#pragma once
#include"Cell.h"

class Fluid
{
public:
	int Nx;
	int Ny;
	float cellSize;
	std::vector<std::vector<Cell>> cells;
	std::vector<std::vector<float>> density;
	std::vector<std::vector<float>> density0;
	std::vector<std::vector<float>> vx;
	std::vector<std::vector<float>> vx0;
	std::vector<std::vector<float>> vy;
	std::vector<std::vector<float>> vy0;

	std::vector<std::vector<float>> div;
	std::vector<std::vector<float>> p;


	Fluid(int Nx, int Ny);

	void integrate(float dt);
	void input(GLFWwindow* window, int screenWidth, int screenHeight, float dt);
	void draw(Shader& render2d, float aspectRatio);
	static glm::vec2 getMouse(GLFWwindow* window, int screenWidth, int screenHeight);
private:
	const float diffuseRate = 0.00011;
	const float visc = 0.001;
	const float mouseSpeed = 500.0f;
	glm::vec2 prevMouse;

	void diffuse(std::vector<std::vector<float>>& grid, std::vector<std::vector<float>>& grid0, int b, float dt, float k);
	void advect(std::vector<std::vector<float>>& grid, std::vector<std::vector<float>>& grid0, std::vector<std::vector<float>>& vx,  
		std::vector<std::vector<float>>& vy, int b, float dt);
	void project();
	void border(int b, std::vector<std::vector<float>>& grid);
	void densityStep(float dt);
	void velocityStep(float dt);
};