#version 330

layout (location = 0) in vec3 Position;
layout (location = 1) in vec4 aColor;
uniform mat4 gWorld;

out vec4 vColor;

void main()
{
    gl_Position = gWorld * vec4(Position.x, Position.y, Position.z, 1.0);
	vColor=aColor;
}