#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in vec4 inBones;
layout(location = 5) in vec4 inWeights;
layout(location = 6) in vec3 inTangent;

layout(location = 0) out vec3 outNormal;
layout(location = 1) out vec2 outUV;
layout(location = 2) out vec3 outColor;
layout(location = 3) out vec4 outWorldPos;
layout(location = 4) out vec3 outTangent;

layout(std140, push_constant) uniform CameraPushConstant {
    mat4 view_proj;
    vec4 pos;
} PushConstants;

layout(std140, binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 bones[128];
} ubo;

void main() {
    outWorldPos = ubo.model * vec4(inPosition, 1.0);
    gl_Position = PushConstants.view_proj * outWorldPos;
    
    outUV = inTexCoord;
    
	mat3 mNormal = transpose(inverse(mat3(ubo.model)));
	outNormal = mNormal * normalize(inNormal);
    outTangent = mNormal * normalize(inTangent);

    outColor = inColor;
}