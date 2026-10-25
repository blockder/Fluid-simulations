#version 460 core

out vec4 FragColor;

in vec2 texCoord;

uniform vec3 color;
uniform float opacity;
uniform sampler2D texUnit;
uniform bool hasTexture;

void main()
{	
	vec4 textureSample = vec4(1);
	if(hasTexture)
		textureSample = texture(texUnit, texCoord);

	FragColor = textureSample * vec4(color, opacity);
}