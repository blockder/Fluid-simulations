#pragma once
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<string>
#include<sstream>
#include<fstream>
#include<iostream>
#include<vector>

std::string get_file_contents(const char* filename);


class Shader
{
public:
	GLuint id;

	// Constructor that build the Shader Program from 2 different shaders
	Shader(const char* vertexFile, const char* fragmentFile);
	Shader(const char* vertexFile, const char* geometryFile, const char* fragmentFile);
	Shader() = default;

	// Activates the Shader Program
	void Active();

	// Deletes the Shader Program
	void Delete();
private:

	// Checks if the different Shaders have compiled properly
	void compileErrors(unsigned int shader, const char* type);

};