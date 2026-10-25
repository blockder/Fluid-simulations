#include"Partical.h"
#include <cmath>

const int Partical::gamma = 7;
const float Partical::k = 2.0;
const float Partical::restDensity = 10;
const float Partical::viscosity = 0.1;
const float Partical::surfaceTensionCoefficient = 5.0;
float velocityDamping = 0.5;
#define SCALE 5

#define poly6 1
#define spiky 2
#define custom 3


Partical::Partical(float mass, glm::vec2 position, float size, GLuint textureId)
{
	this->mass = mass;
	this->position = position;
	this->velocity = glm::vec2(0);
	this->netForce = glm::vec2(0);
	border = false;

	mesh = Mesh(position, size, size, glm::vec3(0), textureId);
}



void Partical::integrate(float dt)
{
	if (border)
		return;
	glm::vec2 acceleration = netForce / density;
	velocity += acceleration * dt;
	position += velocity * dt * velocityDamping;

	netForce = glm::vec2(0);
}

void Partical::draw(Shader& render2d, float aspectRatio)
{
	mesh.position = this->position;
	mesh.draw(render2d, aspectRatio);
}

void Partical::updateDensityPressure(std::vector<Partical>& particals, float radius)
{
	//get density
	density = 0;
	for (int i = 0; i < neighbors.size(); i++)
	{
		Partical& other = particals[neighbors[i]];
		float distance = glm::distance(position, other.position);
		density += other.mass * WeightFunction(distance, radius, poly6) ;
	}
	if (density == 0)
		density = 0.001;

	//calculate pressure
	pressure = k * restDensity / gamma * (std::pow(density / restDensity, gamma) - 1);
	//pressure = k * (density - restDensity);
	if (pressure < 0) pressure = 0;
}

void Partical::pressureForce(std::vector<Partical>& particals, float radius)
{
	for (int i = 0; i < neighbors.size(); i++)
	{
		Partical& other = particals[neighbors[i]];
		float distance = glm::distance(position, other.position);
		if (distance < 0.001) 
			continue;
		glm::vec2 direction = (position - other.position) / distance;
		netForce += direction * other.mass * (pressure + other.pressure) / (2.0f * other.density) * -GradientWeightFunction(distance, radius, spiky);
	}
	if (!border)
	{
		mesh.color = glm::vec3(0, 0, 1) * (restDensity / density + 0.2f);

	}
	
}

void Partical::applyViscosity(std::vector<Partical>& particals, float radius)
{
	glm::vec2 deltaV(0);
	for (int i = 0; i < neighbors.size(); i++)
	{
		Partical& other = particals[neighbors[i]];
		if (other.border)
			continue;
		float distance = glm::distance(position, other.position);
		float w = WeightFunction(distance, radius, poly6);
		float sharedDensity = (density + other.density) / 2.0;
		glm::vec2 relativeVelocity = other.velocity - velocity;
		deltaV += relativeVelocity * other.mass * w / sharedDensity;
	}

	//update velocity
	velocity += deltaV * viscosity;
}

void Partical::surfaceTension(std::vector<Partical>& particals, float radius)
{
	if (border)
		return;

	//calculate surfaceNormal
	surfaceNormal = glm::vec2(0);
	for (int i = 0; i < neighbors.size(); i++)
	{
		Partical& other = particals[neighbors[i]];
		if ( other.density == 0)
			continue;

		float distance = glm::distance(position, other.position);
		if (distance < 0.001)
			continue;
		glm::vec2 direction = (position - other.position) / distance;
		surfaceNormal += direction * -GradientWeightFunction(distance, radius, spiky) * other.mass / other.density;
	}

	if (glm::length(surfaceNormal) > 3.5)
		mesh.color = glm::vec3(0, 1, 0);
	else
	{
		mesh.color = glm::vec3(0, 0, 1) * glm::length(surfaceNormal);
	}
	
	//apply surface tension
	netForce += -surfaceNormal * surfaceTensionCoefficient;
}

float Partical::WeightFunction(float x, float h, int function)
{
	if (x > h)
		return 0;
	x *= SCALE;
	h *= SCALE;
	
	switch (function)
	{
	case poly6:
	{
		float a = 4.0 / (3.14 * std::pow(h, 8));
		return a * std::pow(h*h - x*x, 3);
		break;
	}
	case spiky:
	{
		float a = 10.0 / (3.14 * std::pow(h, 5));
		return a * std::pow(h - x, 3);
		break;
	}
	case custom:
	{
		float volume = 2 * 3.14 * 0.5 * (h + 2 - 2 * (h + 1) * std::log(h + 1) / h);
		return (h - x) / (h * (x + 1)) / volume;
		break;
	}
	}
	std::cout << "did not identify function type" << std::endl;
	return x;
}

float Partical::GradientWeightFunction(float x, float h, int function)
{
	if (x > h)
		return 0;
	x *= SCALE;
	h *= SCALE;
	
	switch (function)
	{
	case poly6:
	{
		float a = -945 / (32 * 3.14 * std::pow(h, 9));
		return a * x * std::pow(h * h - x * x, 2);
		break;
	}
	case spiky:
	{
		float a = -30.0 / (3.14 * std::pow(h, 5));
		return a * std::pow(h - x, 2);
		break;
	}
	case custom:
	{
		return -(h + 1) / (h * (x + 1) * (x + 1));
		break;
	}
	}
	std::cout << "did not identify function type" << std::endl;
	return x;
	
}

