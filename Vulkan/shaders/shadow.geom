#version 450

#define LIGHT_COUNT 6

layout (triangles, invocations = LIGHT_COUNT) in;
layout (triangle_strip, max_vertices = 3) out;

layout (binding = 0) uniform UBO
{
	mat4 mvp[LIGHT_COUNT];
	mat4 instancePos[1024];
} ubo;

layout (location = 0) in int inInstanceIndex[];

void main()
{
	mat4 instancedPos = ubo.instancePos[inInstanceIndex[0]]; 
	for (int i = 0; i < gl_in.length(); i++)
	{
		gl_Layer = gl_InvocationID;
		gl_Position = ubo.mvp[gl_InvocationID] * instancedPos * gl_in[i].gl_Position;
		EmitVertex();
	}
	EndPrimitive();
}