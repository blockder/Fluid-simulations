#include"Fluid.h"

Fluid::Fluid(int Nx, int Ny)
{
	this->Nx = Nx;
	this->Ny = Ny;
	this->cellSize = 1.0 / Nx;
	prevMouse = glm::vec2(0);

	//initialize arrays
	cells.resize(Ny, std::vector<Cell>(Nx));
	density.resize(Ny, std::vector<float>(Nx));
	density0.resize(Ny, std::vector<float>(Nx));
	vx.resize(Ny, std::vector<float>(Nx, 0));
	vx0.resize(Ny, std::vector<float>(Nx, 0));
	vy.resize(Ny, std::vector<float>(Nx, 0));
	vy0.resize(Ny, std::vector<float>(Nx, 0));
	div.resize(Ny, std::vector<float>(Nx));
	p.resize(Ny, std::vector<float>(Nx));
	
	glm::vec2 gridOffset(-1 + cellSize, - Ny * cellSize + cellSize);
	for (int y = 0; y < Ny; y++)
	{
		for (int x = 0; x < Nx; x++)
		{
			float g = 1.0f / (std::rand() % 255);
			float r = 1.0f / (std::rand() % 255);
			float b = 1.0f / (std::rand() % 255);
			glm::vec3 color = glm::normalize(glm::vec3(r, g, b));
			cells[y][x] = Cell(glm::vec2(float(x) * cellSize * 2, float(y) * cellSize * 2) + gridOffset, cellSize, glm::vec3(1, 1, 1));

			//wind tunnel
			vx[y][1] = 200;
			if(y > 15 && y < 35)
			density[y][1] = 1;
			
			//cylinder
			glm::vec2 center(Nx /2 - 20, Ny / 2); float radius = 10;
			float distance = glm::distance(center, glm::vec2(x, y));
			if (distance < radius)
			{
				cells[y][x].wall = true;
				cells[y][x].square.color = glm::vec3(1, 0, 0);
			}
		}	
	}

	
	
}

void Fluid::input(GLFWwindow* window, int screenWidth, int screenHeight, float dt)
{
	//get mouse pos and speed
	glm::vec2 mouse = getMouse(window, screenWidth, screenHeight);
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
		if(!cells[y][x].wall)
		{
			//density[y][x] = 10;
			vx[y][x] += mouseVelocity.x;
			vy[y][x] += mouseVelocity.y;
		}
	
		
		for (int y = 0; y < Ny; y++)
			vx[y][1] = 3;
	}
		
	
	
	

	
	
	
}

void Fluid::integrate(float dt)
{
	velocityStep(dt);
	densityStep(dt);

	///update cell density
	for (int y = 0; y < Ny; y++)
	{
		for (int x = 0; x < Nx; x++)
		{
			cells[y][x].density = density[y][x];
			cells[y][x].velocity = glm::vec2(vx[y][x], vy[y][x]);
			int xc = glm::clamp(x, 1, Nx - 2);
			int yc = glm::clamp(y, 1, Ny - 2);
			float xDiff = vy[yc][xc + 1] - vy[yc][xc - 1];
			float yDiff = vx[yc + 1][xc] - vx[yc - 1][xc];
			float curl = xDiff - yDiff;
			glm::vec3 color(1, 0, 0);
			if (curl > 0)
				color = glm::vec3(0, 0, 1);
			//cells[y][x].square.color = color * std::abs(curl);
		}
	}

}


void Fluid::draw(Shader& render2d, float aspectRatio)
{

	for (std::vector<Cell>& row : cells)
	{
		for (Cell& cell : row)
		{

			cell.square.opacity = cell.density;
			if (cell.wall)
				cell.square.opacity = 1;
			
			float brightnexx = glm::length(cell.velocity);
			//cell.square.opacity = brightnexx;
			cell.square.draw(render2d, aspectRatio);
		}
	}
}







void Fluid::project()
{
	div.clear();
	p.clear();
	div.resize(Ny, std::vector<float>(Nx));
	p.resize(Ny, std::vector<float>(Nx));
	
	//calculate div
	for (int y = 1; y < Ny - 1; y++)
	{
		for (int x = 1; x < Nx - 1; x++)
		{
			float xDiff, yDiff;
			xDiff = vx[y][x + 1] - vx[y][x - 1];
			yDiff = vy[y + 1][x] - vy[y - 1][x];
			div[y][x] = -(xDiff + yDiff) / 2.0f;
			p[y][x] = 0;
		}
	}
	border(0, div); border(0, p);

	//calculate p
	for (int n = 0; n < 30; n++)
	{
		for (int y = 1; y < Ny - 1; y++)
		{
			for (int x = 1; x < Nx - 1; x++)
			{
				float sum = p[y][x + 1] + p[y][x - 1] + p[y + 1][x] + p[y - 1][x];
				p[y][x] = (sum + div[y][x]) / 4.0f;
			}
		}
		border(0, p);
	}
	
	//change velocity
	for (int y = 1; y < Ny - 1; y++)
	{
		for (int x = 1; x < Nx - 1; x++)
		{
			vx[y][x] -= (p[y][x + 1] - p[y][x - 1]) / 2.0;
			vy[y][x] -= (p[y + 1][x] - p[y - 1][x]) / 2.0;
		}
	}
	border(1, vx); border(2, vy);
}

void Fluid::border(int b, std::vector<std::vector<float>>& grid)
{
	//vertical wall
	for (int y = 1; y < Ny - 1; y++)
	{
		grid[y][0] = b == 1 ? -grid[y][1] : grid[y][1];
		grid[y][Nx - 1] = b == 1 ? grid[y][Nx-2] : grid[y][Nx-2];
	}

	//floor
	for (int x = 1; x < Nx - 1; x++)
	{
		grid[0][x] = b == 2 ? -grid[1][x] : grid[1][x];
		grid[Ny-1][x] = b == 2 ? -grid[Ny-2][x] : grid[Ny-2][x];
	}

	//corners
	grid[0][0] = (grid[1][0] + grid[0][1]) / 2.0;
	grid[Ny-1][0] = (grid[Ny-2][0] + grid[Ny-1][1]) / 2.0;
	grid[Ny-1][Nx-1] = (grid[Ny-2][Nx-1] + grid[Ny-1][Nx-2]) / 2.0;
	grid[0][Nx-1] = (grid[1][Nx-1] + grid[0][Nx-2]) / 2.0;

	//wall
	for (int y = 0; y < Ny; y++)
	{
		for (int x = 0; x < Nx; x++)
		{
			if (cells[y][x].wall)
			{
				vx[y][x] = 0;
				vy[y][x] = 0;
			}
		}
	}
}
glm::vec2 Fluid::getMouse(GLFWwindow* window, int screenWidth, int screenHeight)
{
	float aspectRatio = float(screenWidth) / screenHeight;
	double mouseX, mouseY;
	glfwGetCursorPos(window, &mouseX, &mouseY);

	//step 1 normalize
	mouseX /= screenWidth;
	mouseY /= screenHeight;

	//step 2 multiply
	mouseX *= 2;
	mouseY *= -2;

	//step 3 move
	mouseX -= 1;
	mouseY += 1;

	//step 4.5 aspect ratio
	mouseX *= aspectRatio;

	return glm::vec2(mouseX, mouseY);
}

void Fluid::diffuse(std::vector<std::vector<float>>& grid, std::vector<std::vector<float>>& grid0, int b, float dt, float diff)
{
	return;
	float k = diff * dt * Nx * Ny;
	for (int n = 0; n < 20; n++)
	{
		for (int y = 1; y < Ny - 1; y++)
		{
			for (int x = 1; x < Nx - 1; x++)
			{
				float sum = grid[y][x + 1] + grid[y][x - 1] + grid[y + 1][x] + grid[y - 1][x];
				grid[y][x] = (grid0[y][x] + k * sum) / (1 + 4 * k);
			}
		}
		border(b, grid);
	}

}

void Fluid::advect(std::vector<std::vector<float>>& grid, std::vector<std::vector<float>>& grid0, std::vector<std::vector<float>>& vx,
	std::vector<std::vector<float>>& vy, int b, float dt)
{
	for (int y = 1; y < Ny - 1; y++)
	{
		for (int x = 1; x < Nx - 1; x++)
		{
			float xPos = x - vx[y][x] * dt * Nx;
			float yPos = y - vy[y][x] * dt * Ny;
			xPos = glm::clamp(xPos, 1.5f, Nx - 1.5f);
			yPos = glm::clamp(yPos, 0.5f, Ny - 1.5f);
			int ix = int(xPos), iy = int(yPos);
			float s = xPos - ix, t = yPos - iy;
			float up = (1 - s) * grid0[iy + 1][ix] + s * grid0[iy + 1][ix + 1];
			float down = (1 - s) * grid0[iy][ix] + s * grid0[iy][ix + 1];
			grid[y][x] = (1 - t) * down + t * up;

		}
	}
	border(b, grid);
}

void Fluid::densityStep(float dt)
{
	density0 = density;
	diffuse(density, density0, 0, dt, diffuseRate);
	density0 = density;
	advect(density, density0, vx, vy, 0, dt);

}

void Fluid::velocityStep(float dt)
{
	vx0 = vx;	diffuse(vx, vx0, 1, dt, visc);
	vy0 = vy;	diffuse(vy, vy0, 2, dt, visc);
	project();
	vx0 =vx; vy0 = vy;
	advect(vx, vx0, vx0, vy0, 1, dt);
	advect(vy, vy0, vx0, vy0, 2, dt);
	project();
}
