#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out vec3 viewPos;
layout(location = 4) out vec3 fragPos;

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main() {
	vec4 vertPos = ubo.model * vec4(inPosition, 1.0);
    gl_Position = ubo.proj * ubo.view * vertPos;
    fragColor = inColor;
    fragTexCoord = inTexCoord;
	fragNormal = inNormal;
	viewPos = vec3(ubo.view[0][3], ubo.view[1][3], ubo.view[2][3]);
	fragPos = vertPos.xyz;
}