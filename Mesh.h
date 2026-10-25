#pragma once
#include"Shader.h"
#include<glm/glm.hpp>
#include<vector>

struct Vertex
{
	glm::vec2 position;
	glm::vec2 texCoord;
	Vertex(glm::vec2 position, glm::vec2 texCoord)
		:position(position), texCoord(texCoord) {
	}
};
class Mesh
{
	friend class ParticalSystem;
public:
	glm::vec2 position;
	glm::vec2 scale;
	glm::vec3 color;
	float opacity;

	Mesh() = default;
	Mesh(const Mesh& other);
	Mesh(glm::vec2 position, float width, float height, glm::vec3 color);
	Mesh(glm::vec2 position, float width, float height, glm::vec3 color, GLuint textureId);

	void draw(Shader& render2d, float aspectRatio);
	static GLuint loadTexture(const char* textureName);

private:
	std::vector<glm::vec2> points;
	std::vector<Vertex> vertices;

	//opengl stuff
	GLuint VAO;
	GLuint VBO;
	GLuint Texture;
	bool hasTexture;

	void createSquare();
	void loadVBO();
};