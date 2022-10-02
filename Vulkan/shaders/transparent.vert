#version 450

layout (location = 0) in vec4 inPos;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec3 inNormal;
layout (location = 3) in vec2 inUV;

layout(std140, binding = 0) readonly buffer InstanceData {
    mat4 model[];
} ssbo;

layout (binding = 1) uniform UBO {
    mat4 view_proj;
} ubo;

layout (location = 0) out vec3 outColor;
layout (location = 1) out vec2 outUV;

void main () 
{
	outColor = inColor;
	outUV = inUV;
	
	gl_Position = ubo.view_proj * ssbo.model[gl_InstanceIndex] * vec4(inPos.xyz, 1.0);		
}