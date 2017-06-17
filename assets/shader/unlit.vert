#version 420
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 vertexUV;
layout(location = 3) in vec4 vertexColor;

uniform mat4 MVPMatrix;
uniform mat4 ModelViewMatrix;
uniform mat4 ModelMatrix;

out vec2 texCoord;
out vec4 vertColor;

void main( void )
{
	texCoord = vertexUV;
	vertColor = vertexColor;
	gl_Position = MVPMatrix * vec4(vertexPosition,1);
};