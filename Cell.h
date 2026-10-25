#pragma once
#include"Mesh.h"

class Cell
{
public:
	bool wall;
	glm::vec2 velocity;
	float density;
	Mesh square;

	Cell()
	{
		velocity = glm::vec2(0);
		density = 0;
	}
	Cell(glm::vec2 position, float size, glm::vec3 color)
	{
		wall = false;
		this->velocity = glm::vec2(0);
		this->density = 0;
		this->square = Mesh(position, size, size, color);
	}
};