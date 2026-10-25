#include"ParticalSystem.h"

ParticalSystem::ParticalSystem(Mesh shape, GLuint texture)
{
	this->shape = shape;
	this->Texture = texture;
	this->shape.loadVBO();

	//create VAO, and VBO
	glCreateVertexArrays(1, &VAO);
	glCreateBuffers(1, &particalVBO);

	//connect VBO to VAO
	glVertexArrayVertexBuffer(VAO, 0, this->shape.VBO, 0, sizeof(Vertex)); 
	glVertexArrayVertexBuffer(VAO, 1, particalVBO, 0, sizeof(ParticalState));

	//set layouts
	glEnableVertexArrayAttrib(VAO, 0);													//positions
	glVertexArrayAttribFormat(VAO, 0, 2, GL_FLOAT, GL_FALSE, (GLuint)0);

	glEnableVertexArrayAttrib(VAO, 1);													//texCoord
	glVertexArrayAttribFormat(VAO, 1, 2, GL_FLOAT, GL_FALSE, (GLuint)2 * sizeof(float));

	glEnableVertexArrayAttrib(VAO, 2);													//movement
	glVertexArrayAttribFormat(VAO, 2, 2, GL_FLOAT, GL_FALSE, (GLuint)0 * sizeof(float));

	glEnableVertexArrayAttrib(VAO, 3);													//color
	glVertexArrayAttribFormat(VAO, 3, 3, GL_FLOAT, GL_FALSE, (GLuint)2 * sizeof(float));

	glEnableVertexArrayAttrib(VAO, 4);													//opacity
	glVertexArrayAttribFormat(VAO, 4, 1, GL_FLOAT, GL_FALSE, (GLuint)5 * sizeof(float));

	glEnableVertexArrayAttrib(VAO, 5);													//scale
	glVertexArrayAttribFormat(VAO, 5, 1, GL_FLOAT, GL_FALSE, (GLuint)6 * sizeof(float));

	//set particalVBO to instancing
	glVertexArrayBindingDivisor(VAO, 1, 1);

	//assign layouts to buffers
	glVertexArrayAttribBinding(VAO, 0, 0);
	glVertexArrayAttribBinding(VAO, 1, 0);
	glVertexArrayAttribBinding(VAO, 2, 1);
	glVertexArrayAttribBinding(VAO, 3, 1);
	glVertexArrayAttribBinding(VAO, 4, 1);
	glVertexArrayAttribBinding(VAO, 5, 1);
	

}

void ParticalSystem::draw(Shader& renderPartical, float aspectRatio)
{
	renderPartical.Active();
	glBindVertexArray(VAO);
	int textureUnit = 0;
	glBindTextureUnit(textureUnit, Texture);

	//send texture to shader
	glUniform1i(glGetUniformLocation(renderPartical.id, "texUnit"), textureUnit);

	//send aspectRatio to shader
	glUniform1f(glGetUniformLocation(renderPartical.id, "aspectRatio"), aspectRatio);

	//draw call
	glDrawArraysInstanced(GL_TRIANGLES, 0, shape.vertices.size(), ps.size());

	//unbind VAO and texture
	glBindVertexArray(0);
	glBindTextureUnit(textureUnit, 0);
}
void ParticalSystem::loadParticals(std::vector<Partical>& particals)
{
	ps.clear();
	for (Partical& partical : particals)
	{
		if (partical.border)
			continue;
		ParticalState pstate;
		pstate.position = partical.position;
		pstate.color = partical.mesh.color;
		pstate.opacity = partical.mesh.opacity;
		pstate.scale = partical.mesh.scale.x;
		ps.push_back(pstate);
	}

	//put data in VBO 
	glNamedBufferData(particalVBO, sizeof(ParticalState) * ps.size(), ps.data(), GL_DYNAMIC_DRAW);
}
