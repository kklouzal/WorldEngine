#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
	mat4 bones[128];
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in ivec4 inBones;
layout(location = 5) in vec4 inWeights;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 fragNormal;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main() {
	mat4 boneTransform = ubo.bones[inBones[0]] * inWeights[0];
	boneTransform     += ubo.bones[inBones[1]] * inWeights[1];
	boneTransform     += ubo.bones[inBones[2]] * inWeights[2];
	boneTransform     += ubo.bones[inBones[3]] * inWeights[3];

    gl_Position = ubo.proj * ubo.view * ubo.model * boneTransform * vec4(inPosition, 1.0);
    fragColor = inColor;
    fragTexCoord = inTexCoord;
}