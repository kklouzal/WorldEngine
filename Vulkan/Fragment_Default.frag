#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 viewPos;
layout(location = 4) in vec3 fragPos;

layout(location = 0) out vec4 outColor;
layout(binding = 1) uniform sampler2D texSampler;

layout(binding = 2) uniform UniformBufferObject_PointLights {
    vec3 position[64];
	vec3 ambient[64];
	vec3 diffuse[64];
    vec3 CLQ[64];
	uint count;
} lights;

vec3 CalcPointLight(uint lnum, vec3 normal) {
    vec3 lightDir = normalize(lights.position[lnum] - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // attenuation
    float distance		= length(lights.position[lnum] - fragPos);
    float attenuation	= 1.0 / (lights.CLQ[lnum].x + lights.CLQ[lnum].y * distance + 
						lights.CLQ[lnum].z * (distance * distance));    
    // combine results
    vec3 ambient = lights.ambient[lnum] * vec3(texture(texSampler, fragTexCoord));
    vec3 diffuse = lights.diffuse[lnum] * diff * vec3(texture(texSampler, fragTexCoord));
    ambient *= attenuation;
    diffuse *= attenuation;
    return (ambient + diffuse);
}

void main() {
    vec3 norm = normalize(fragNormal);

	vec3 point_Result = vec3(0,0,0);

	for(int i = 0; i < lights.count; i++) {
		point_Result += CalcPointLight(i, norm);
	}

   outColor = vec4(point_Result, 1.0);
}