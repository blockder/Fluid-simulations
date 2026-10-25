#pragma once
#include"Cell.h"

class BLM
{
public:
	int Nx;
	int Ny;
	float cellSize;
	std::vector<std::vector<Cell>> cells;
	std::vector<std::vector<std::vector<float>>> F;
	std::vector<std::vector<std::vector<float>>> F0;

	BLM(int Nx, int Ny);

	void integrate(float dt);
	void input(GLFWwindow* window, int screenWidth, int screenHeight, float dt);

	void draw(Shader& render2, float aspectRatio);
private:
	glm::vec2 prevMouse;
	float mouseSpeed = 500;
	std::vector<int> cxs;
	std::vector<int> cys;
	std::vector<float> weights;

	glm::vec2 getVelocity(int x, int y);
	float getDensity(int x, int y);

};