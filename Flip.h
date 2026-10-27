#pragma once
#include<vector>
#include"ParticalSystem.h"



namespace FL { class Partical; };


class Flip
{
public:
	int Nx;
	int Ny;
	float cellSize;
	ParticalSystem particalSystem;
	std::vector<FL::Partical> particals;
	std::vector<std::vector<float>> vx;
	std::vector<std::vector<float>> vx0;
	std::vector<std::vector<float>> wx;
	std::vector<std::vector<float>> vy;
	std::vector<std::vector<float>> vy0;
	std::vector<std::vector<float>> wy;
	std::vector<std::vector<float>> s;
	std::vector<std::vector<int>> cellType;
	std::vector<std::vector<float>> density;

	Flip(int Nx, int Ny, int numParticals);

	void integrate(float dt);
	void input(GLFWwindow* window, int screenWidth, int screenHeight, float dt);
	void draw(Shader& particalShader, float aspectRatio);
	void project();
	void border();
	void transferToGrid(glm::vec2 position, float q, int gridType);
	float getSample(glm::vec2 position, int gridType);
	void updateGrid();
	void updateParticals();
	float isValid(int x, int y, int gridType);
	void collision();

private:
	const float overRelaxation = 1.5;
	const float restDensity = 0.0;
	const float k = 0.02;
	const float flipRatio = 0.9;
	float pRadius;
	glm::vec2 prevMouse;
	glm::vec2 worldToVx(glm::vec2 worldPosition);
	glm::vec2 worldToVy(glm::vec2 worldPosition);
	glm::vec2 screenToWorld(glm::vec2 screenPosition);
	
};

namespace FL
{
	class Partical
	{
	public:
		glm::vec2 position;
		glm::vec2 velocity;
		glm::vec3 color;
		float scale;
		float mass;

		Partical() = default;
		Partical(glm::vec2 position, float mass, float scale);

		ParticalState getState();

	
	private:
	};

};