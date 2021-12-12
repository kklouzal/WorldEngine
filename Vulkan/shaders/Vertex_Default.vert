#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in vec4 inBones;
layout(location = 5) in vec4 inWeights;
layout(location = 6) in vec3 inTangent;

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec2 outUV;
layout (location = 2) out vec3 outColor;
layout (location = 3) out vec3 outWorldPos;
layout (location = 4) out vec3 outTangent;

layout(push_constant) uniform constants
{
    mat4 view;
    mat4 proj;
} PushConstants;

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    mat4 bones[128];
} ubo;

void main() {
	vec4 vertPos = ubo.model * vec4(inPosition, 1.0);
    gl_Position = PushConstants.proj * PushConstants.view * vertPos;

    outColor = inColor;
    outUV = inTexCoord;
	outNormal = inNormal;
	outWorldPos = vertPos.xyz;
    outTangent = inTangent;
}