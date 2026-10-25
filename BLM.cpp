#include"BLM.h"
#include"Fluid.h"
#include <cmath>

BLM::BLM(int Nx, int Ny)
{
	this->Nx = Nx;
	this->Ny = Ny;
	cellSize = 1.0 / Nx;

	//initialize arrays
	F.resize(Ny, std::vector<std::vector<float>>(Nx, std::vector<float>(9, 0)));
	F0.resize(Ny, std::vector<std::vector<float>>(Nx, std::vector<float>(9, 0)));
	cells.resize(Ny, std::vector<Cell>(Nx));

	cxs = {0, 0, 1, 1, 1, 0, -1, -1, -1};
	cys = {0, 1, 1, 0, -1, -1, -1, 0, 1};
	weights = {4.0/9, 1.0/9, 1.0/36, 1.0/9, 1.0/36, 1.0/9, 1.0/36, 1.0/9, 1.0/36};

	glm::vec2 gridOffset(-1 + cellSize, -Ny * cellSize + cellSize);
	for (int y = 0; y < Ny; y++)
	{
		for (int x = 0; x < Nx; x++)
		{
			float g = 1.0f / (std::rand() % 255);
			float r = 1.0f / (std::rand() % 255);
			float b = 1.0f / (std::rand() % 255);
			glm::vec3 color = glm::normalize(glm::vec3(r, g, b));
			cells[y][x] = Cell(glm::vec2(float(x) * cellSize * 2, float(y) * cellSize * 2) + gridOffset, cellSize, glm::vec3(1, 1, 1));
			//F[y][x][3] = 5;
			for (int i = 0; i < 9; i++)
			{
				F[y][x][i] = 1;
			}
			F[y][x][3] = 2.3;

			//cylinder
			glm::vec2 center(Ny / 2.0, Nx / 4.0);
			if (glm::distance(glm::vec2(x, y), center) < 5)
			{
				cells[y][x].wall = true;
			}
		}
	}
	
}
void BLM::input(GLFWwindow* window, int screenWidth, int screenHeight, float dt)
{
	//get mouse pos and speed
	glm::vec2 mouse = Fluid::getMouse(window, screenWidth, screenHeight);
	glm::vec2 mouseVelocity = (mouse - prevMouse) * mouseSpeed;
	prevMouse = mouse;

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
	{
		glm::vec2 pos = mouse;
		glm::vec2 offset(1, Ny * cellSize);
		glm::vec2 scale(Nx / 2, Nx / 2);
		pos += offset;
		pos *= scale;

		int x = glm::clamp((int)pos.x, 0, Nx - 1);
		int y = glm::clamp((int)pos.y, 0, Ny - 1);
		if (!cells[y][x].wall)
		{
			F[y][x][3] += 2;

		}

		

	}
}
void BLM::integrate(float dt)
{
	//stream
	//border
	for (int y = 0; y < Ny; y++)
	{
		for (int i : {2, 3, 4})
		F[y][0][i] = F[y][1][i];
	
		for (int i : {6, 7, 8})
		F[y][Nx - 1][i] = F[y][Nx - 2][i];
	}
	F0 = F;
	int opposite[9] = {0, 5, 6, 7, 8, 1, 2, 3, 4};
	for (int y = 0; y < Ny; y++)
	{
		for (int x = 0; x < Nx; x++)
		{
			for (int i = 0; i < 9; i++)
			{
				//cell to update
				int nx = x + cxs[i];
				int ny = y + cys[i];

				//border check
				if (nx < 0) nx = Nx - 1; if (nx > Nx - 1) nx = 0;
				if (ny < 0) ny = Ny - 1; if (ny > Ny - 1) ny = 0;

				//update next cell
				F[ny][nx][i] = F0[y][x][i];
				
				//wall
				if (cells[ny][nx].wall)
				{
					F[ny][nx][i] = F0[y][x][opposite[i]];
				}


			}
		}
				
	} 
	
	//collision
	for (int y = 0; y < Ny; y++)
	{
		for (int x = 0; x < Nx; x++)
		{
			//density
			float p = getDensity(x, y);
			
			//momentum
			glm::vec2 velocity = getVelocity(x, y);
			float ux = velocity.x;
			float uy = velocity.y;
			
			//update F
			for (int i = 0; i < 9; i++)
			{
				float du = cxs[i] * ux + cys[i] * uy;
				float Feq = p * weights[i] * (1 + (3 * du) + (9 * du * du / 2) - (3 * (ux*ux + uy*uy) / 2));
				float fCurrent = F[y][x][i];
				F[y][x][i] = fCurrent - (fCurrent - Feq) / 0.53;
			}
		}
	}
}

void BLM::draw(Shader& render2d, float aspectRatio)
{
	for (int y = 0; y < Ny; y++)
	{
		for (int x = 0; x < Nx; x++)
		{
			glm::vec3 color(1);
			/*if (x > 0 && x < Nx - 1 && y > 0 && y < Ny - 1)
			{
				float yDiff = getVelocity(x, y + 1).x - getVelocity(x, y - 1).x;
				float xDiff = getVelocity(x+1, y).y - getVelocity(x-1, y).y;
				float curl = xDiff - yDiff;
				color = glm::vec3(1, 0, 0);
				if (curl < 0)
					color = glm::vec3(0, 0, 1);
				cells[y][x].square.opacity = std::abs(curl)*5 ;
			}*/

			
			cells[y][x].square.color = color;
			cells[y][x].square.opacity = glm::length(getVelocity(x, y));
			cells[y][x].square.draw(render2d, aspectRatio);
		}
	}
}

glm::vec2 BLM::getVelocity(int x, int y)
{
	float p = getDensity(x, y);
	float ux = 0, uy = 0;
	if (!cells[y][x].wall)
	{
		for (int i = 0; i < 9; i++)
		{
			ux += cxs[i] * F[y][x][i];
			uy += cys[i] * F[y][x][i];
		}
		if (p > 0)
		{
			ux /= p;
			uy /= p;
		}
	}
	return glm::vec2(ux, uy);
}

float BLM::getDensity(int x, int y)
{
	//density
	float p = 0;
	for (int i = 0; i < 9; i++)
		p += F[y][x][i];

	return p;
}
