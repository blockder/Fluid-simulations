#include"SPH.h"
#include"Fluid.h"
#include <chrono>
#define cell(coord, cellSize) (std::floor(coord / cellSize))

SPH::SPH(int numParticals)
{
	GLuint texture = Mesh::loadTexture("partical.png");
	glm::vec2 position(-0.5, -0.5);
	float spacing = 1.0 / std::sqrt(float(numParticals));
	float particalSize = 0.02;
	for (int n = 0; n < numParticals; n++)
	{
		Partical partical(1, position, particalSize, texture);
		partical.mesh.opacity = 1;
		partical.mesh.color = glm::vec3(1);
		particals.push_back(partical);
		particals.back().neighbors.reserve(100);
		position.x += spacing;
		if (position.x > 0.5)
		{
			position.y += spacing;
			position.x = -0.5;
		}
	}

	initiateBorderParticals(std::sqrt(float(numParticals)), particalSize, texture);
	particalSystem = ParticalSystem(particals[0].mesh, texture);

	//initialize grid
	grid.resize(toChunk(1, particalRadius) + 3);
	for (int i = 0; i < grid.size(); i++)
	{
		grid[i].resize(toChunk(1, particalRadius) + 3);
	}
	
}
void SPH::integrate(float dt)
{
	
	updateNeighbors();

	//calculate density and pressure
	for (Partical& partical : particals)
		partical.updateDensityPressure(particals, particalRadius);

	//apply forces
	for (Partical& partical : particals)
	{
		partical.pressureForce(particals, particalRadius);
		partical.netForce += glm::vec2(0, -1) * 10.0f * partical.density;
		partical.surfaceTension(particals, particalRadius);
		partical.applyViscosity(particals, particalRadius);
	}

	//integrate particals
	for (Partical& partical : particals)
		partical.integrate(dt);

	border();
}


void SPH::updateNeighbors()
{
	float sqrRadius = particalRadius * particalRadius;
	updateGrid();
	int offset[] = {
	0, 0, 
	0, 1,
	1, 1,
	1, 0,
	1, -1,
	0, -1,
	-1, -1,
	-1, 0,
	-1, 1
	};

	for (int i = 0; i < particals.size(); i++)
	{
		particals[i].neighbors.clear();
		particals[i].neighbors.reserve(100);
		glm::vec2 position = particals[i].position;
		int cellX = toChunk(position.x, particalRadius);
		int cellY = toChunk(position.y, particalRadius);
		for (int k = 0; k < 9; k++)
		{
			std::vector<int>& possibleNeighbors = getChunk(cellX + offset[k*2], cellY + offset[k*2 + 1]);
			for (int j = 0; j < possibleNeighbors.size(); j++)
			{
				int otherI = possibleNeighbors[j];
				if (i == otherI)
					continue;
				glm::vec2 r = position - particals[otherI].position;
				if (glm::dot(r, r) < sqrRadius)
					particals[i].neighbors.emplace_back(otherI);
			}
		}
	}
	
}
void SPH::updateGrid()
{
	//cler grid
	for (int x = 0; x < grid.size(); x++)
	{
		for(int y=0; y < grid[x].size(); y++)
		grid[x][y].clear();
	}
		
		
	//re add particals
	for (int i = 0; i < particals.size(); i++)
	{
		glm::vec2 pos = particals[i].position;
		int cellX = toChunk(pos.x, particalRadius);
		int cellY = toChunk(pos.y, particalRadius);
		grid[cellX][cellY].push_back(i);
	}
}
void SPH::draw(Shader& renderPartical, float aspectRatio)
{
	particalSystem.loadParticals(particals);
	particalSystem.draw(renderPartical, aspectRatio);
}

void SPH::border()
{
	for (Partical& partical : particals)
	{
		//floor
		if (partical.position.y < -1)
		{
			partical.position.y = -1;
			partical.velocity.y *= -0.9f;
		}
		//ceiling
		if (partical.position.y > 1)
		{
			partical.position.y = 1;
			partical.velocity.y *= -0.9f;
		}
		//left wall
		if (partical.position.x < -1)
		{
			partical.position.x = -1;
			partical.velocity.x *= -0.9f;
		}
		//right wall
		if (partical.position.x > 1)
		{
			partical.position.x = 1;
			partical.velocity.x *= -0.9f;
		}

		
	}
}
void SPH::initiateBorderParticals(int numParticals, float particalSize, GLuint texture)
{ 
	numParticals *= 5;
	float spacing = 2.0 / numParticals;

	//floor
	float x = -1;
	for (int i = 0; i < numParticals; i++)
	{
		Partical partical(1, glm::vec2(x, -1), particalSize, texture);
		partical.mesh.color = glm::vec3(0, 1, 0);
		partical.border = true;
		particals.push_back(partical);
		x += spacing;
	}

	//walls
	float y = -1;
	for (int i = 0; i < numParticals; i++)
	{
		Partical partical1(1, glm::vec2(1, y), particalSize, texture);
		partical1.mesh.color = glm::vec3(0, 1, 0);
		partical1.border = true;
		Partical partical2(1, glm::vec2(-1, y), particalSize, texture);
		partical2.mesh.color = glm::vec3(0, 1, 0);
		partical2.border = true;
		particals.push_back(partical1);
		particals.push_back(partical2);
		y += spacing;
	}


}
std::vector<int>& SPH::getChunk(glm::vec2 coord, float cellSize)
{
	int cellX = toChunk(coord.x, cellSize);
	int cellY = toChunk(coord.y, cellSize);
	return getChunk(cellX, cellY);
}
std::vector<int>& SPH::getChunk(int cellX, int cellY)
{
	return grid[cellX][cellY];
}
int SPH::toChunk(float coord, float cellSize)
{
	int cell = std::floor(coord / cellSize);
	int edge = std::floor(1 / cellSize);
	return cell + edge + 2;
	
}
void SPH::input(GLFWwindow* window, int screenWidth, int screenHeight)
{
	//get mouse pos and speed
	glm::vec2 mouse = Fluid::getMouse(window, screenWidth, screenHeight);
	glm::vec2 mouseVelocity = (mouse - prevMouse) * mouseSpeed;
	prevMouse = mouse;

	//get all close particals
	std::vector<int> inRange;
	for (int i = 0; i< particals.size(); i++)
	{
		if (glm::distance(particals[i].position, mouse) < 0.2)
		{
			inRange.push_back(i);
		}
	}

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
	{
		for (int i : inRange)
		{
			Partical& p = particals[i];
			//p.velocity += mouseVelocity;
			p.velocity += glm::normalize(p.position - mouse) * 1.0f;
		}
	}
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
	{
		for (int i : inRange)
		{
			Partical& p = particals[i];
			p.velocity -= glm::normalize(p.position - mouse) * 1.0f;
		}
	}
}