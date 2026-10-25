#pragma once
#include"Cell.h"

class Fluid3
{
public:
	int Nx;
	int Ny;
	float cellSize;
	std::vector<std::vector<Cell>> cells;
	std::vector <std::vector<float>> vx;
	std::vector <std::vector<float>> vx0;
	std::vector <std::vector<float>> vy;
	std::vector <std::vector<float>> vy0;
	std::vector <std::vector<float>> density;
	std::vector <std::vector<float>> density0;
	std::vector <std::vector<float>> s;

	Fluid3(int Nx, int Ny);

	void integrate(float dt);
	void input(GLFWwindow* window, int screenWidth, int screenHeight, float dt);

	void draw(Shader& render2d, float aspectRatio);
private:
	glm::vec2 prevMouse;
	glm::vec2 cylinder;
	float mouseSpeed = 1000;
	const float overRelaxation = 1.7;

	void project();
	void advectVelocity(float dt);
	void advectDensity(float dt);
	void border();
	float getSample(float x, float y, int field);
	void setCircle(glm::vec2 center, float radious);
	
};