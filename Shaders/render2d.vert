#version 460 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTex;


out vec2 texCoord;

uniform mat4 movement;
uniform mat4 scale;
uniform float aspectRatio;

void main()
{
	texCoord = aTex;
	gl_Position = movement * scale * vec4(aPos, 0.0f, 1.0f) * vec4(aspectRatio, 1, 1, 1.0f);
}