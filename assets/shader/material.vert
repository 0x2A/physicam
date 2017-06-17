#version 420
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 vertexUV;
layout(location = 3) in vec4 vertexColor;

uniform mat4 MVPMatrix;
uniform mat4 ModelViewMatrix;
uniform mat4 ModelMatrix;
uniform vec2 UVTile;

out vec2 texCoord;
out vec4 vertColor;
out vec3 vertPos;
out vec3 vertNormal;

void main( void )
{
	texCoord = vertexUV * UVTile;
	vertColor = vertexColor;
	vertNormal = normalize((ModelViewMatrix * vec4(vertexNormal,0)).xyz);
	vec4 vertPos4 = ModelViewMatrix * vec4(vertexPosition,1);
	vertPos = vec3(vertPos4) / vertPos4.w;
	gl_Position = MVPMatrix * vec4(vertexPosition,1);
};