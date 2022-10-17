#version 450

#define LIGHT_COUNT 6

layout (triangles, invocations = LIGHT_COUNT) in;
layout (triangle_strip, max_vertices = 3) out;

layout (location = 0) in mat4 inInstanceMat[]; //  location 0,1,2,3

layout (binding = 0) uniform UBO
{
	mat4 mvp[LIGHT_COUNT];
} ubo;

void main()
{
	for (int i = 0; i < gl_in.length(); i++)
	{
		gl_Layer = gl_InvocationID;
		gl_Position = ubo.mvp[gl_InvocationID] * inInstanceMat[0] * gl_in[i].gl_Position;
		EmitVertex();
	}
	EndPrimitive();
}