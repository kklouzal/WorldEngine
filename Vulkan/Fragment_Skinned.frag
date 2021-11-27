#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 viewPos;
layout(location = 4) in vec3 fragPos;

layout(location = 0) out vec4 outColor;
layout(binding = 1) uniform sampler2D texSampler;

void main() {
	vec4 lightColor = vec4(8,8,8,10);
	float ambientStrength = 0.5;
	vec4 ambient = ambientStrength * lightColor;

	vec4 result = ambient * texture(texSampler, fragTexCoord);
    outColor =  result;
}