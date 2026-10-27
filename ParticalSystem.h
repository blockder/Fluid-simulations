#pragma once
#include<glm/glm.hpp>
#include"Mesh.h"

class Partical;
struct ParticalState
{
	glm::vec2 position;
	glm::vec3 color;
	float opacity;
	float scale;

};
class ParticalSystem
{
public:
	std::vector<ParticalState> ps;
	Mesh shape;

	ParticalSystem() = default;
	ParticalSystem(Mesh shape, GLuint texture);

	void loadParticals(std::vector<Partical>& particals);
	void loadParticals(std::vector<ParticalState>& ps);
	void draw(Shader& renderPartical, float aspectRatio);
private:
	GLuint VAO;
	GLuint particalVBO;
	GLuint Texture;

};