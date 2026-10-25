#pragma once
#include"Partical.h"
#include"ParticalSystem.h"

class SPH
{
public:
	std::vector<Partical> particals;
	ParticalSystem particalSystem;

	SPH(int numParticals);

	void integrate(float dt);
	void input(GLFWwindow* window, int screenWidth, int screenHeight);

	void draw(Shader& renderPartical, float aspectRatio);
private:
	const float particalRadius = 0.2;
	glm::vec2 prevMouse;
	const float mouseSpeed = 10;
	std::vector<std::vector<std::vector<int>>> grid;

	void updateNeighbors();
	void updateGrid();
	void border();
	void initiateBorderParticals(int numParticals, float particalSize, GLuint texture);
	std::vector<int>& getChunk(glm::vec2 coord, float cellSize);
	std::vector<int>& getChunk(int cellX, int cellY);
	static int toChunk(float coord, float cellSize);
	
};