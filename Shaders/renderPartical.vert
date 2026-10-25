#version 460 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTex;
layout (location = 2) in vec2 aMovement;
layout (location = 3) in vec3 aColor;
layout (location = 4) in float aOpacity;
layout (location = 5) in float aScale;


out vec2 texCoord;
out vec3 color;
out float opacity;



uniform float aspectRatio;

void main()
{
	texCoord = aTex;
	color = aColor;
	opacity = aOpacity;
	gl_Position = (vec4(aMovement,0,0) + vec4(aPos * aScale, 0.0f, 1.0f)) * vec4(aspectRatio, 1, 1, 1.0f);
}