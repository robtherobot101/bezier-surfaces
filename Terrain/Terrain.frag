#version 330

in vec4 theColour;
in vec2 texCoord;
in float height;
in float ripple;
uniform sampler2D grassTex;
uniform sampler2D rockTex;
uniform sampler2D snowTex;
uniform sampler2D iceTex;
uniform vec4 texBounds;
uniform float waterLevel;

vec4 white = vec4(1.0);
float GRASS_MAX = 10;
float ROCK_MAX = 20;
vec4 waterCol;

void main() 
{
	if (waterLevel < texBounds[2])
	{
		waterCol = ripple * vec4(0.1, 0.5, 0.7, 0.0);
	}
	else
	{
		waterCol = texture(iceTex, texCoord);
	}
	float water = 1 - clamp(6 * (height - waterLevel), 0, 1);
	float grass = clamp(2 * (-abs(height - (texBounds[0] + texBounds[1]) / 2) + (texBounds[1] - texBounds[0]) / 2) + 0.5, 0, 1);
	float rock = clamp(2 * (-abs(height - (texBounds[1] + texBounds[2]) / 2) + (texBounds[2] - texBounds[1]) / 2) + 0.5, 0, 1);
	float snow = clamp(2 * (-abs(height - (texBounds[2] + texBounds[3]) / 2) + (texBounds[3] - texBounds[2]) / 2) + 0.5, 0, 1);
    gl_FragColor = theColour * (grass * texture(grassTex, texCoord)
								+ rock  * texture(rockTex,  texCoord)
								+ snow  * texture(snowTex,  texCoord)
								) * (1 - water) + water * waterCol;
}
