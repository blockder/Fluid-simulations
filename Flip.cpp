#include"Flip.h"
#include"Fluid.h"

#define VX_FIELD 4
#define VY_FIELD 5
#define DENSITY 6
#define FLIP_X 7
#define FLIP_Y 8
#define FLUID_CELL 1
#define SOLID_CELL 0
#define AIR_CELL 2

Flip::Flip(int Nx, int Ny, int numParticals)
{
	this->Nx = Nx;
	this->Ny = Ny;
	cellSize = 2.0 / Nx;
	pRadius = 0.3 * cellSize;
	particalSystem = ParticalSystem(Mesh(glm::vec2(0), 1, 1, glm::vec3(0)), Mesh::loadTexture("partical.png"));

	//initialize grids
	vx.resize(Nx + 1, std::vector<float>(Ny));
	vx0.resize(Nx + 1, std::vector<float>(Ny));
	wx.resize(Nx + 1, std::vector<float>(Ny, 0));
	vy.resize(Nx, std::vector<float>(Ny + 1));
	vy0.resize(Nx, std::vector<float>(Ny + 1));
	wy.resize(Nx, std::vector<float>(Ny + 1, 0));
	s.resize(Nx, std::vector<float>(Ny));
	cellType.resize(Nx, std::vector<int>(Ny));
	density.resize(Nx, std::vector<float>(Ny));

	//initialize particals
	glm::vec2 position(-0.8, -0.5);
	float spacing = 1.0 / std::sqrt(float(numParticals));
	for (int n = 0; n < numParticals; n++)
	{
		FL::Partical partical(position, 1, pRadius);
		partical.color = glm::vec3(0, 0, 1);
		particals.push_back(partical);
		position.x += spacing;
		if (position.x > 0.8)
		{
			position.y += spacing;
			position.x = -0.5;
		}
	}
	particals[0].position = glm::vec2(0,0);

	//initialize cell
	for (int x = 0; x < Nx; x++)
	{
		for (int y = 0; y < Ny; y++)
		{
			s[x][y] = 1;
			if (x == 0 || y == 0 || x == Nx - 1)
			{
				s[x][y] = 0;
			}
		}
	}
}
void Flip::input(GLFWwindow* window, int screenWidth, int screenHeight, float dt)
{
	//get mouse pos and speed
	glm::vec2 mouse = Fluid::getMouse(window, screenWidth, screenHeight);
	glm::vec2 mouseVelocity = (mouse - prevMouse) * 10.0f;
	prevMouse = mouse;

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
	{
		glm::vec2 pos = screenToWorld(mouse);
		
		for (FL::Partical& partical : particals)
		{
			glm::vec2 ppos = partical.position;
			if (glm::distance(mouse, ppos) < 0.2)
			{
				partical.velocity = mouseVelocity * 5.0f;
			}
		}
		

	}
}
void Flip::integrate(float dt)
{
	//integrate particals
	for (FL::Partical& partical : particals)
	{
		partical.velocity += glm::vec2(0, -1) * 10.0f * dt;
		partical.position += partical.velocity * dt;
	}
	collision();
	border();

	//transfer properties to grid
	updateGrid();


	//make velocity field incompressible
	project();

	//transfer properties back to particals
	updateParticals();

}
void Flip::updateGrid()
{
	//reset 
	vx0 = vx;
	vy0 = vy;

	vx.assign(Nx + 1, std::vector<float>(Ny, 0.0f));
	wx.assign(Nx + 1, std::vector<float>(Ny, 0.0f));

	vy.assign(Nx, std::vector<float>(Ny + 1, 0.0f));
	wy.assign(Nx, std::vector<float>(Ny + 1, 0.0f));
	density.assign(Nx, std::vector<float>(Ny, 0));


	//reset cellType
	for (int x = 0; x < Nx; x++)
	{
		for (int y = 0; y < Ny; y++)
		{
			cellType[x][y] = s[x][y] == 0.0 ? SOLID_CELL : AIR_CELL;

		}
	}

	//update cell type
	for (FL::Partical& partical : particals)
	{
		glm::vec2 worldPosition = screenToWorld(partical.position);
		int ix = worldPosition.x;
		int iy = worldPosition.y;

		//mark as fluid cell
		if (cellType[ix][iy] == AIR_CELL)
			cellType[ix][iy] = FLUID_CELL;
	}

	//transfer velocities
	for (FL::Partical& partical : particals)
	{
		glm::vec2 worldPosition = screenToWorld(partical.position);
		//transfer vx
		transferToGrid(worldPosition, partical.velocity.x, VX_FIELD);

		//transfer vy
		transferToGrid(worldPosition, partical.velocity.y, VY_FIELD);

		//add density
		transferToGrid(worldPosition, 1, DENSITY);
	}

	//normalize velicities
	//vx
	for (int x = 0; x < Nx + 1; x++)
		for (int y = 0; y < Ny; y++)
			if (wx[x][y] > 0)
				vx[x][y] /= wx[x][y];
	//vy
	for (int x = 0; x < Nx; x++)
		for (int y = 0; y < Ny + 1; y++)
			if (wy[x][y] > 0)
				vy[x][y] /= wy[x][y];

	//restore solid
	for (int x = 0; x < Nx; x++)
	{
		for (int y = 0; y < Ny; y++)
		{
			if (cellType[x][y] == SOLID_CELL)
			{
				vx[x][y] = vx0[x][y];
				vx[x + 1][y] = vx0[x + 1][y];
				vy[x][y] = vy0[x][y];
				vy[x][y+1] = vy0[x][y+1];
			}
		}
	}

}
void Flip::updateParticals()
{
	for (FL::Partical& partical : particals)
	{
		glm::vec2 worldPosition = screenToWorld(partical.position);

		//pic
		float picVx = getSample(worldPosition, VX_FIELD);
		float picVy = getSample(worldPosition, VY_FIELD);

		//flip
		float flipVx = partical.velocity.x + getSample(worldPosition, FLIP_X);
		float flipVy = partical.velocity.y + getSample(worldPosition, FLIP_Y);

		//update velocty
		partical.velocity.x = (1 - flipRatio) * picVx + flipRatio * flipVx;
		partical.velocity.y = (1 - flipRatio) * picVy + flipRatio * flipVy;
	}

}
float Flip::isValid(int x, int y, int gridType)
{
	x = std::min(x, Nx - 1);
	y = std::min(y, Ny - 1);
	if (gridType == VX_FIELD)
	{
		if (cellType[x][y] != AIR_CELL || cellType[std::max(x - 1,0)][y] != AIR_CELL)
			return 1;
		return 0;
	}
	else if (gridType == VY_FIELD)
	{
		if (cellType[x][y] != AIR_CELL || cellType[x][std::max(y-1,0)] != AIR_CELL)
			return 1;
		return 0;
	}
	else
	{
		if (cellType[x][y] != AIR_CELL)
			return 1;
		return 0;
	}
}
void Flip::collision()
{
	float rMin = pRadius * 2;
	for(int iter=0; iter <4; iter++)
	for (int i=0; i<particals.size(); i++)
	{
		for (int j = 0; j < particals.size(); j++)
		{
			if (i == j)
				continue;
			glm::vec2 r = particals[i].position - particals[j].position;
			if (glm::dot(r, r) <= rMin * rMin && glm::dot(r, r) > 0)
			{
				float distance = glm::length(r);
				float length = rMin - distance;
				particals[i].position += r / distance / 2.0f * length;
				particals[j].position -= r / distance / 2.0f * length;
			}
		}
	}
}
void Flip::draw(Shader& particalShader, float aspectRatio)
{
	std::vector<ParticalState> ps;
	for (FL::Partical& partical : particals)
	{
		ps.push_back(partical.getState());
	}
	particalSystem.loadParticals(ps);
	particalSystem.draw(particalShader, aspectRatio);
}

FL::Partical::Partical(glm::vec2 position, float mass, float scale)
{
	this->position = position;
	this->mass = mass;
	this->scale = scale;
	velocity = glm::vec2(0);
	color = glm::vec3(1);

}

ParticalState FL::Partical::getState()
{
	ParticalState state;
	state.position = position;
	state.color = color;
	state.opacity = 1;
	state.scale = scale;
	return state;
}

void Flip::project()
{
	vx0 = vx;
	vy0 = vy;
	for (int n = 0; n < 20; n++)
	{
		for (int x = 1; x < Nx - 1; x++)
		{
			for (int y = 1; y < Ny - 1; y++)
			{
				if (cellType[x][y] != FLUID_CELL)
					continue;
				float xDiff = vx[x + 1][y] - vx[x][y];
				float yDiff = vy[x][y + 1] - vy[x][y];
				float div = xDiff + yDiff;
				float pressure = k * (density[x][y] - restDensity);
				div -= pressure;

				float s1 = s[x + 1][y];
				float s2 = s[x - 1][y];
				float s3 = s[x][y + 1];
				float s4 = s[x][y - 1];
				float sTotal = s1 + s2 + s3 + s4;
				if (sTotal == 0)
					continue;

				//update velocities
				float p = div * overRelaxation / sTotal;
				vx[x + 1][y] -= s1 * p;
				vx[x][y] += s2 * p;
				vy[x][y + 1] -= s3 * p;
				vy[x][y] += s4 * p;
			}
		}
	}
}
void Flip::border()
{
	for (FL::Partical& partical : particals)
	{
		//floor
		float minY = -Ny * cellSize / 2.0f + pRadius  + cellSize;
		if (partical.position.y < minY )
		{
			partical.position.y = minY;
			partical.velocity.y = 0.0f;
		}
		//ceiling
		float maxY = Ny * cellSize / 2.0f;
		if (partical.position.y > maxY)
		{
			partical.position.y = maxY;
			partical.velocity.y = 0.0f;
		}
		//left wall
		float xMin = -1 + pRadius + cellSize;
		if (partical.position.x < xMin)
		{
			partical.position.x = xMin;
			partical.velocity.x = 0.0f;
		}
		//right wall
		float xMax = 1 - pRadius - cellSize;
		if (partical.position.x > xMax)
		{
			partical.position.x = xMax;
			partical.velocity.x = 0.0f;
		}

	}

	
}

void Flip::transferToGrid(glm::vec2 position, float q, int gridType)
{
	//set grid
	std::vector<std::vector<float>>* grid = nullptr;
	std::vector<std::vector<float>>* w = nullptr;
	switch (gridType)
	{
	case VX_FIELD:
		grid = &vx;
		w = &wx;
		position = worldToVx(position);
		break;
	case VY_FIELD:
		grid = &vy;
		w = &wy;
		position = worldToVy(position);
		break;
	case DENSITY:
		grid = &density;
		position.x -= 0.5;
		position.y -= 0.5;
		position.x = glm::clamp(position.x, 1.0f, Nx - 1.1f);
		position.y = glm::clamp(position.y, 1.0f, Ny - 1.1f);
	}

	//variables
	float x = position.x;
	float y = position.y;
	int ix0 = int(x), ix1 = ix0 + 1;
	int iy0 = int(y), iy1 = iy0 + 1;
	if (ix1 >= grid->size()) ix1 = grid->size() - 1;
	if (iy1 >= grid->at(0).size()) iy1 = grid->at(0).size() - 1;
	float tx = x - ix0;
	float ty = y - iy0;
	float w1 = (1 - tx) * (1 - ty);
	float w2 = tx * (1 - ty);
	float w3 = (1 - tx) * ty;
	float w4 = tx * ty;

	
	//add to each corner
	(*grid)[ix0][iy0] += w1 * q;
	(*grid)[ix1][iy0] += w2 * q;	
	(*grid)[ix0][iy1] += w3 * q;	
	(*grid)[ix1][iy1] += w4 * q;	
	if (w)
	{
		(*w)[ix0][iy0] += w1;
		(*w)[ix1][iy0] += w2;
		(*w)[ix0][iy1] += w3;
		(*w)[ix1][iy1] += w4;

	}

}

float Flip::getSample(glm::vec2 position, int gridType)
{
	std::vector<std::vector<float>>* f = nullptr;
	std::vector<std::vector<float>>* f0 = nullptr;

	//select field
	switch (gridType)
	{
	case VX_FIELD:
		f = &vx;
		position = worldToVx(position);
		break;
	case VY_FIELD:
		f = &vy;
		position = worldToVy(position);
		break;
	case DENSITY:
		f = &density;
		position.x = glm::clamp(position.x, 0.0f, Nx - 1.1f);
		position.y = glm::clamp(position.y, 0.0f, Ny - 1.1f);
		break;
	case FLIP_X:
		f = &vx;
		f0 = &vx0;
		position = worldToVx(position);
		gridType = VX_FIELD;
		break;
	case FLIP_Y:
		f = &vy;
		f0 = &vy0;
		position = worldToVy(position);
		gridType = VY_FIELD;
		break;
	}

	//variables
	float x = position.x;
	float y = position.y;
	int ix0 = int(x), ix1 = ix0 + 1;
	int iy0 = int(y), iy1 = iy0 + 1;
	if (ix1 >= f->size()) ix1 = f->size() - 1;
	if (iy1 >= f->at(0).size()) iy1 = f->at(0).size() - 1;
	float tx = x - ix0;
	float ty = y - iy0;
	float valid1 = isValid(ix0, iy0, gridType);
	float valid2 = isValid(ix1, iy0, gridType);
	float valid3 = isValid(ix0, iy1, gridType);
	float valid4 = isValid(ix1, iy1, gridType);
	float w1 = (1 - tx) * (1 - ty) * valid1;
	float w2 = tx * (1 - ty) * valid2;
	float w3 = (1 - tx) * ty * valid3;
	float w4 = tx * ty * valid4;
	float weightSum = w1 + w2 + w3 + w4;

	//bilinear intepolation
	float result = 0;
	if (weightSum > 0)
	{
		float c1 = (*f)[ix0][iy0];
		float c2 = (*f)[ix1][iy0];
		float c3 = (*f)[ix0][iy1];
		float c4 = (*f)[ix1][iy1];
		if (f0)
		{
			c1 -= (*f0)[ix0][iy0];
			c2 -= (*f0)[ix1][iy0];
			c3 -= (*f0)[ix0][iy1];
			c4 -= (*f0)[ix1][iy1];
		}
		result = c1 * w1 + c2 * w2 + c3 * w3 + c4 * w4;
		result /= weightSum;

	}
	
	return result;
}

glm::vec2 Flip::worldToVx(glm::vec2 worldPosition)
{
	glm::vec2 localPosition;
	worldPosition.y -= 0.5;
	localPosition.x = glm::clamp(worldPosition.x, 0.0f, Nx - 0.1f);
	localPosition.y = glm::clamp(worldPosition.y, 0.0f, Ny - 1.1f);
	return localPosition;
}

glm::vec2 Flip::worldToVy(glm::vec2 worldPosition)
{
	glm::vec2 localPosition;
	worldPosition.x -= 0.5;
	localPosition.x = glm::clamp(worldPosition.x, 0.0f, Nx - 1.1f);
	localPosition.y = glm::clamp(worldPosition.y, 0.0f, Ny - 0.1f);
	return localPosition;
}

glm::vec2 Flip::screenToWorld(glm::vec2 screenPosition)
{
	glm::vec2 worldPosition;
	glm::vec2 offset(1, Ny * cellSize / 2.0);
	glm::vec2 scale(Nx / 2, Nx / 2);
	screenPosition += offset;
	screenPosition *= scale;
	worldPosition.x = glm::clamp(screenPosition.x, 0.0f, Nx - 0.1f);
	worldPosition.y = glm::clamp(screenPosition.y, 0.0f, Ny - 0.1f);
	return worldPosition;
}


