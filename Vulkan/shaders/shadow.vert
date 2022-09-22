#version 450

layout (location = 0) in vec3 inPos;

layout (location = 0) out int outInstanceIndex;

void main()
{
	outInstanceIndex = gl_InstanceIndex;
	gl_Position = vec4(inPos, 1.0);
}