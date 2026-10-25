#pragma once
#include<glm/glm.hpp>
#include<vector>
#include"Mesh.h"

class Partical
{
public:
	glm::vec2 position;
	glm::vec2 velocity;
	glm::vec2 netForce;
	float mass;
	std::vector<int> neighbors;
	Mesh mesh;
	float density;
	float pressure;
	glm::vec2 surfaceNormal;
	bool border;


	Partical() = default;
	Partical(float mass, glm::vec2 position, float size, GLuint texture);

	void integrate(float dt);

	void draw(Shader& render2d, float aspctRatio);
	void updateDensityPressure(std::vector<Partical>& particals, float radius);
	void pressureForce(std::vector<Partical>& particals, float radius);
	void applyViscosity(std::vector<Partical>& particals, float radius);
	void surfaceTension(std::vector<Partical>& particals, float radius);
	static float WeightFunction(float x, float h, int function);
	static float GradientWeightFunction(float x, float h, int function);

private:
	const static float k;
	const static float restDensity;
	const static int gamma;
	const static float viscosity;
	const static float 	surfaceTensionCoefficient;
	;

};