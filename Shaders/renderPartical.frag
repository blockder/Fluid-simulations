#version 460 core

out vec4 FragColor;

in vec2 texCoord;
in vec3 color;
in float opacity;

uniform sampler2D texUnit;


void main()
{	
	

	FragColor = texture(texUnit, texCoord) * vec4(color, opacity);
}