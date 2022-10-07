#version 450

#define LIGHT_COUNT 6

layout (triangles, invocations = LIGHT_COUNT) in;
layout (triangle_strip, max_vertices = 3) out;

layout (binding = 0) uniform UBO
{
	mat4 mvp[LIGHT_COUNT];
} ubo;

//layout (location = 0) in mat4 inInstanceMat[]; //  location 0,1,2,3
layout(location = 0) in vec4 inMatX[];
layout(location = 1) in vec4 inMatY[];
layout(location = 2) in vec4 inMatZ[];
layout(location = 3) in vec4 inMatW[];
mat4 inInstanceMat = mat4(inMatX[0], inMatY[0], inMatZ[0], inMatW[0]);

void main()
{
	for (int i = 0; i < gl_in.length(); i++)
	{
		gl_Layer = gl_InvocationID;
		gl_Position = ubo.mvp[gl_InvocationID] * inInstanceMat * gl_in[i].gl_Position;
		EmitVertex();
	}
	EndPrimitive();
}