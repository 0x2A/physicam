#version 330
uniform vec4 DiffuseColor;
uniform sampler2D Texture;

in vec2 texCoord;
in vec4 vertColor;

out vec4 colorOut;

void main( void )
{
	colorOut = DiffuseColor * texture(Texture, texCoord);
};