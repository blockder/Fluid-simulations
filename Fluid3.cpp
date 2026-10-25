#include"Fluid3.h"
#include"Fluid.h"
#define VX_FIELD 1
#define VY_FIELD 2
#define DENSITY_FIELD 3

Fluid3::Fluid3(int Nx, int Ny)
{
	this->Nx = Nx;
	this->Ny = Ny;
	cellSize = 1.0 / Nx;
	cylinder = glm::vec2(Nx / 2, Ny / 2);

	//initialize arrays
	cells.resize(Nx, std::vector<Cell>(Ny));
	vx.resize(Nx+1, std::vector<float>(Ny));
	vx0.resize(Nx+1, std::vector<float>(Ny));
	vy.resize(Nx, std::vector<float>(Ny+1));
	vy0.resize(Nx, std::vector<float>(Ny+1));
	density.resize(Nx, std::vector<float>(Ny, 0));
	density0.resize(Nx, std::vector<float>(Ny, 0));
	s.resize(Nx, std::vector<float>(Ny));


	glm::vec2 gridOffset(-1 + cellSize, -Ny * cellSize + cellSize);
	for (int x = 0; x < Nx; x++)
	{
		for (int y = 0; y < Ny; y++)
		{
			cells[x][y] = Cell(glm::vec2(float(x) * cellSize * 2, float(y) * cellSize * 2) + gridOffset, cellSize, glm::vec3(1, 1, 1));
			s[x][y] = 1;
			if (x == 0 || y == 0 || y == Ny - 1)
			{
				s[x][y] = 0;
				cells[x][y].square.opacity = 0;
			}
			
			//wind tunnel
			if (x == 1)
			{
				if(y > 15 && y < 35)
				density[0][y] = 1;
				vx[x][y] = 50;

			}
		}
	}
	vx0 = vx;
	vy0 = vy;
	density0 = density;

	

}
void Fluid3::input(GLFWwindow* window, int screenWidth, int screenHeight, float dt)
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
		if (s[x][y] != 0)
		{
			/*density[x][y] = 1;
			vy[x][y] = mouseVelocity.y;
			vy[x][y+1] = mouseVelocity.y;
			vx[x][y] = mouseVelocity.x;
			vx[x+1][y] = mouseVelocity.x;*/
		}
		setCircle(glm::vec2(x, y), 5);
		
	}
}
void Fluid3::integrate(float dt)
{
	project();
	border();
	advectVelocity(dt);
	advectDensity(dt);

}

void Fluid3::draw(Shader& render2d, float aspectRatio)
{
	for (int x = 0; x < Nx; x++)
	{
		for (int y = 0; y < Ny; y++)
		{
			if(s[x][y] != 0)
			cells[x][y].square.opacity = density[x][y];
			cells[x][y].square.draw(render2d, aspectRatio);
		}
	}
}

void Fluid3::project()
{
	for (int n = 0; n < 20; n++)
	{
		for (int x = 1; x < Nx - 1; x++)
		{
			for (int y = 1; y < Ny - 1; y++)
			{
				if (s[x][y] == 0)
					continue;

				float xDiff = vx[x + 1][y] - vx[x][y];
				float yDiff = vy[x][y + 1] - vy[x][y];
				float div = xDiff + yDiff;

				float sTotal = s[x + 1][y] + s[x - 1][y] + s[x][y + 1] + s[x][y - 1];
				if (sTotal == 0)
					continue;

				//update velocities
				float p = div * overRelaxation / sTotal;
				vx[x + 1][y] -= s[x + 1][y] * p;
				vx[x][y] += s[x-1][y] * p;
				vy[x][y+1] -= s[x][y+1] * p;
				vy[x][y] += s[x][y-1] * p;
			}
		}
	}
}
void Fluid3::border()
{
	//wall
	for (int y = 0; y < Ny; y++)
	{
		vy[0][y] = vy[1][y];
		vy[Nx - 1][y] = vy[Nx - 2][y];
	}

	//floor
	for (int x = 0; x < Nx; x++)
	{
		vx[x][0] = vx[x][1];
		vx[x][Ny - 1] = vx[x][Ny - 2];
	}
}
float Fluid3::getSample(float x, float y, int field)
{
	std::vector<std::vector<float>>* f = nullptr;

	//select feild
	if (field == 1)			f = &vx0;
	else if (field == 2)	f = &vy0;
	else if (field == 3)	f = &density0;

	//variables
	int ix0 = int(x), ix1 = ix0 + 1;
	int iy0 = int(y), iy1 = iy0 + 1;
	if (ix1 >= f->size()) ix1 = f->size() - 1;
	if (iy1 >= f->at(0).size()) iy1 = f->at(0).size() - 1;
	float tx = x - ix0;
	float ty = y - iy0;
	float bl = (*f)[ix0][iy0];
	float br = (*f)[ix1][iy0];
	float tl = (*f)[ix0][iy1];
	float tr = (*f)[ix1][iy1];

	//bilinear intepolation
	float up = tl * (1 - tx) + tr * tx;
	float down = bl * (1 - tx) + br * tx;
	return up * ty + down * (1 - ty);

}
void Fluid3::setCircle(glm::vec2 center, float radious)
{
	//erase previous cylinder
	for (int x = std::max(0.0f, cylinder.x - radious); x < std::min(float(Nx), cylinder.x + radious); x++)
	{
		for (int y = std::max(0.0f, cylinder.y - radious); y < std::min(float(Ny), cylinder.y + radious); y++)
		{
			s[x][y] = 1;
			cells[x][y].square.color = glm::vec3(1);
		}
		
	}

	//create new cylinder
	this->cylinder = center;
	for (int x = std::max(0.0f, cylinder.x - radious); x < std::min(float(Nx),cylinder.x + radious); x++)
	{
		for (int y = std::max(0.0f, cylinder.y - radious); y < std::min(float(Ny), cylinder.y + radious); y++)
		{
			if (glm::distance(cylinder, glm::vec2(x, y)) < radious)
			{
				s[x][y] = 0;
				density[x][y] = 0;
				vx[x][y] = 0;
				cells[x][y].square.color = glm::vec3(1, 1,0);
				cells[x][y].square.opacity = 1;
			}
		}
	}
}
void Fluid3::advectVelocity(float dt)
{
	//update vx
	vx0 = vx;
	for (int x = 1; x < Nx; x++)
	{
		for (int y = 0; y < Ny; y++)
		{
			if (s[x][y] == 0 || s[x - 1][y] == 0)
				continue;
			float u = vx0[x][y];
			float v = getSample(x - 0.5, y + 0.5, VY_FIELD);
			float xPrev = x - u * dt;
			float yPrev = y - v * dt;
			if (xPrev > Nx - 1)
			{
				int g = 0;
			}
			xPrev = glm::clamp(xPrev, 0.0f, Nx - 0.1f);
			yPrev = glm::clamp(yPrev, 0.0f, Ny - 1.1f);

			//update velocity
			vx[x][y] = getSample(xPrev, yPrev, VX_FIELD);
		}
	}

	//update vy
	vy0 = vy;
	for (int x = 0; x < Nx; x++)
	{
		for (int y = 1; y < Ny; y++)
		{
			if (s[x][y] == 0 || s[x][y-1] == 0)
				continue;
			float u = getSample(x + 0.5, y - 0.5, VX_FIELD);
			float v = vy0[x][y];
			float xPrev = x - u * dt;
			float yPrev = y - v * dt;
			xPrev = glm::clamp(xPrev, 0.0f, Nx - 1.1f);
			yPrev = glm::clamp(yPrev, 0.0f, Ny - 0.1f);

			//update velocity
			vy[x][y] = getSample(xPrev, yPrev, VY_FIELD);
		}
	}

}
void Fluid3::advectDensity(float dt)
{
	//update density
	density0 = density;
	for (int x = 0; x < Nx; x++)
	{
		for (int y = 0; y < Ny; y++)
		{
			if (s[x][y] == 0)
				continue;
			float u = getSample(x + 0.5, y, VX_FIELD);
			float v = getSample(x, y + 0.5, VY_FIELD);
			float xPrev = x  - u * dt;
			float yPrev = y  - v * dt;
			xPrev = glm::clamp(xPrev, 0.0f, Nx - 0.1f);
			yPrev = glm::clamp(yPrev, 0.0f, Ny - 0.1f);

			//update density
			density[x][y] = getSample(xPrev, yPrev, DENSITY_FIELD);
		}
	}
}


