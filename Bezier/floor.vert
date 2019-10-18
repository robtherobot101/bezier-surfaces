#version 330

layout (location = 0) in vec2 position;
uniform mat4 mvpMatrix;

void main()
{
	gl_Position = mvpMatrix * vec4(position.x, 0, position.y, 1);

}
