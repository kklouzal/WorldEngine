#version 450

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
    mat4 bones[128];
} ubo;

void main() {
	vec4 vertPos = ubo.model * vec4(inPosition, 1.0);
    gl_Position = PushConstants.proj * PushConstants.view * vertPos;
    
    outUV = inTexCoord;

	outWorldPos = vertPos.xyz;
    
	mat3 mNormal = transpose(inverse(mat3(ubo.model)));
	outNormal = mNormal * normalize(inNormal);
    outTangent = mNormal * normalize(inTangent);

    outColor = inColor;
}