#version 420
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 vertexUV;
layout(location = 3) in vec4 vertexColor;

uniform mat4 MVPMatrix;

out vec2 texCoord;

void main( void )
{
	texCoord = vertexUV;
	gl_Position = MVPMatrix * vec4(vertexPosition,1);
};