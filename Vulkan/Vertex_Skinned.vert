#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in ivec4 inBones;
layout(location = 5) in vec4 inWeights;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out vec3 viewPos;
layout(location = 4) out vec3 fragPos;

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
	mat4 bones[128];
} ubo;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main() {
	vec4 totalPosition = vec4(0.0f);
	for (int i = 0; i < 4; i++)
	{
		if(boneIds[i] == 0)
		{
            continue;
		}
		if (inBones[i] >= 128)
		{
			totalPosition = vec4(inPosition, 1.0f);
			break;
		}

		vec4 localPosition = ubo.bones[inBones[i]] * vec4(inPosition,1.0f);
		totalPosition += localPosition * inWeights[i];
		vec3 localNormal = mat3(ubo.bones[inBones[i]]) * inNormal;
	}

	mat4 viewModel = ubo.view * ubo.model;
	gl_Position = ubo.proj * viewModel * totalPosition;

    fragColor = inColor;
    fragTexCoord = inTexCoord;
	fragNormal = inNormal;
	viewPos = vec3(ubo.view[0][3], ubo.view[1][3], ubo.view[2][3]);
	fragPos = gl_Position.xyz;
}