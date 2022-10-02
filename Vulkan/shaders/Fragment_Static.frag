#version 450

layout(binding = 2) uniform sampler2D samplerColor;
layout(binding = 3) uniform sampler2D samplerNormalMap;

layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec2 inUV;
//layout(location = 2) in vec3 inColor;
layout(location = 3) in vec4 inWorldPos;
layout(location = 4) in vec3 inTangent;

//	Attachments out
layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outPosition;
layout(location = 2) out vec3 outNormal;
layout(location = 3) out vec4 outAlbedo;

layout (constant_id = 0) const float NEAR_PLANE = 0.1f;
layout (constant_id = 1) const float FAR_PLANE = 256.0f;
float linearDepth(float depth)
{
	float z = depth * 2.0f - 1.0f; 
	return (2.0f * NEAR_PLANE * FAR_PLANE) / (FAR_PLANE + NEAR_PLANE - z * (FAR_PLANE - NEAR_PLANE));	
}

void main() {
	outPosition = inWorldPos;
	outAlbedo = texture(samplerColor, inUV);

	// Calculate normal in tangent space
	vec3 N = normalize(inNormal);
	vec3 T = normalize(inTangent);
	outNormal = mat3(T, cross(N, T), N) * normalize(texture(samplerNormalMap, inUV).xyz * 2.0 - vec3(1.0));

	outPosition.a = linearDepth(gl_FragCoord.z);

	//	Write color attachment to avoid undefined behaviour
	outColor = vec4(0.0);
}