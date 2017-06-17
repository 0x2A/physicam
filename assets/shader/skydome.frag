#version 330
uniform sampler2D Texture;
uniform float brightness;

in vec2 texCoord;

out vec4 colorOut;

void main( void )
{
	vec4 color = texture(Texture, texCoord);
	
	//see SOIL documentation: SOIL_HDR_RGBdivA: RGB / A
	colorOut = vec4((color.xyz / color.w) * brightness, 1);
	colorOut = pow(colorOut, vec4(2.2f));
	
};