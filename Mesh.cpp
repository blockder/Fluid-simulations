#include"Mesh.h"
#include<glm/gtc/type_ptr.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include"stb/stb_image.h"

Mesh::Mesh(glm::vec2 position, float width, float height, glm::vec3 color, GLuint textureId) : Mesh(position, width, height, color)
{
	this->Texture = textureId;
	hasTexture = true;
}
Mesh::Mesh(glm::vec2 position, float width, float height, glm::vec3 color)
{
	this->position = position;
	scale = glm::vec2(width, height);
	this->color = color;
	opacity = 1.0f;
	this->Texture = 0;
	hasTexture = false;

	//create VAO, and VBO
	glCreateVertexArrays(1, &VAO);
	glCreateBuffers(1, &VBO);

	//connect VBO to VAO
	glVertexArrayVertexBuffer(VAO, 0, VBO, 0, sizeof(Vertex)); //connects VBO to buffer-slot 0 in VAO

	//set layouts
	glEnableVertexArrayAttrib(VAO, 0);													//positions
	glVertexArrayAttribFormat(VAO, 0, 2, GL_FLOAT, GL_FALSE, (GLuint)0);
	glEnableVertexArrayAttrib(VAO, 1);													//texCoord
	glVertexArrayAttribFormat(VAO, 1, 2, GL_FLOAT, GL_FALSE, (GLuint)2 * sizeof(float));

	//assign layouts to buffer 0 
	glVertexArrayAttribBinding(VAO, 0, 0);
	glVertexArrayAttribBinding(VAO, 1, 0);

	createSquare();
	loadVBO();
}


Mesh::Mesh(const Mesh& other) : Mesh(other.position, other.scale.x, other.scale.y, other.color, other.Texture)
{

}
void Mesh::draw(Shader& render2d, float aspectRatio)
{
	render2d.Active();
	glBindVertexArray(VAO);
	int textureUnit = 0;
	glBindTextureUnit(textureUnit, Texture);

	//send texture to shader
	glUniform1i(glGetUniformLocation(render2d.id, "texUnit"), textureUnit);

	//send hasTexture
	glUniform1i(glGetUniformLocation(render2d.id, "hasTexture"), hasTexture);

	//send color to shader
	glUniform3fv(glGetUniformLocation(render2d.id, "color"), 1, glm::value_ptr(color));

	//send opacity to shader
	glUniform1f(glGetUniformLocation(render2d.id, "opacity"), opacity);

	//send aspectRatio to shader
	glUniform1f(glGetUniformLocation(render2d.id, "aspectRatio"), aspectRatio);

	//send movement to shader
	glUniformMatrix4fv(glGetUniformLocation(render2d.id, "movement"), 1, GL_FALSE, glm::value_ptr(glm::translate(glm::mat4(1.0f), glm::vec3(position, 0))));

	//send scale to shader
	glUniformMatrix4fv(glGetUniformLocation(render2d.id, "scale"), 1, GL_FALSE, glm::value_ptr(glm::scale(glm::mat4(1), glm::vec3(scale, 1))));

	//draw call
	glDrawArrays(GL_TRIANGLES, 0, vertices.size());

	//unbind VAO and texture
	glBindVertexArray(0);
	glBindTextureUnit(textureUnit, 0);
}

GLuint Mesh::loadTexture(const char* textureName)
{
	//load bytes form file
	int width, height, numColch;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* bytes = stbi_load(textureName, &width, &height, &numColch, 0);
	if (!bytes)
		throw std::runtime_error("Failed to load texture");

	//create texture
	GLuint Texture;
	glCreateTextures(GL_TEXTURE_2D, 1, &Texture);

	//set parameters
	glTextureParameteri(Texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTextureParameteri(Texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(Texture, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTextureParameteri(Texture, GL_TEXTURE_WRAP_T, GL_REPEAT);

	//put image into texture object
	int mipmapLevel = 4;
	if (numColch == 4)
	{
		glTextureStorage2D(Texture, mipmapLevel, GL_RGBA8, width, height);
		glTextureSubImage2D(Texture, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, bytes);
	}
	else if (numColch == 3)
	{
		glTextureStorage2D(Texture, mipmapLevel, GL_RGB8, width, height);
		glTextureSubImage2D(Texture, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, bytes);
	}
	else if (numColch == 2)
	{
		glTextureStorage2D(Texture, mipmapLevel, GL_RG8, width, height);
		glTextureSubImage2D(Texture, 0, 0, 0, width, height, GL_RG, GL_UNSIGNED_BYTE, bytes);
	}
	else if (numColch == 1)
	{
		glTextureStorage2D(Texture, mipmapLevel, GL_R8, width, height);
		glTextureSubImage2D(Texture, 0, 0, 0, width, height, GL_RED, GL_UNSIGNED_BYTE, bytes);
	}
	else throw std::invalid_argument("automatic texture type recognition falied");

	glGenerateTextureMipmap(Texture);

	stbi_image_free(bytes);

	return Texture;
}

void Mesh::createSquare()
{
	//create points
	points.clear();
	points.emplace_back(-1, -1);
	points.emplace_back(-1, 1);
	points.emplace_back(1, 1);
	points.emplace_back(1, -1);

	//create vertices
	vertices.clear();
	vertices.emplace_back(points[0], glm::vec2(0, 0));
	vertices.emplace_back(points[1], glm::vec2(0, 1));
	vertices.emplace_back(points[2], glm::vec2(1, 1));

	vertices.emplace_back(points[2], glm::vec2(1, 1));
	vertices.emplace_back(points[3], glm::vec2(1, 0));
	vertices.emplace_back(points[0], glm::vec2(0, 0));
}

void Mesh::loadVBO()
{
	//put data in VBO 
	glNamedBufferData(VBO, sizeof(Vertex) * vertices.size(), vertices.data(), GL_DYNAMIC_DRAW);
}