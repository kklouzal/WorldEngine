#version 450

#define LIGHT_COUNT 6
#define SHADOW_FACTOR 0.25
#define AMBIENT_LIGHT 0.33
#define USE_PCF
//#define USE_SHADOWS

layout(binding = 2) uniform sampler2D samplerposition;
layout(binding = 3) uniform sampler2D samplerNormal;
layout(binding = 4) uniform sampler2D samplerAlbedo;
layout (binding = 6) uniform sampler2DArray samplerShadowMap;

layout(location = 0) in vec2 inUV;

layout(location = 0) out vec4 outFragcolor;

struct DLight {
	vec4 position;
	vec4 color;
	float radius;
	mat4 viewMatrix;
};

layout(std140, push_constant) uniform CameraPushConstant {
    mat4 view_proj;
    vec4 pos;
} PushConstants;

layout(std140, binding = 5) uniform DComposition
{
	DLight lights[6];
	int displayDebugTarget;
} ubo;

float textureProj(vec4 P, float layer, vec2 offset)
{
	float shadow = 1.0;
	vec4 shadowCoord = P / P.w;
	shadowCoord.st = shadowCoord.st * 0.5 + 0.5;
	
	if (shadowCoord.z > -1.0 && shadowCoord.z < 1.0) 
	{
		float dist = texture(samplerShadowMap, vec3(shadowCoord.st + offset, layer)).r;
		if (shadowCoord.w > 0.0 && dist < shadowCoord.z) 
		{
			shadow = SHADOW_FACTOR;
		}
	}
	return shadow;
}

float filterPCF(vec4 sc, float layer)
{
	ivec2 texDim = textureSize(samplerShadowMap, 0).xy;
	float scale = 1.5;
	float dx = scale * 1.0 / float(texDim.x);
	float dy = scale * 1.0 / float(texDim.y);

	float shadowFactor = 0.0;
	int count = 0;
	int range = 1;
	
	for (int x = -range; x <= range; x++)
	{
		for (int y = -range; y <= range; y++)
		{
			shadowFactor += textureProj(sc, layer, vec2(dx*x, dy*y));
			count++;
		}
	
	}
	return shadowFactor / count;
}

vec3 shadow(vec3 fragcolor, vec3 fragpos) {
	for(int i = 0; i < LIGHT_COUNT; ++i)
	{
		vec4 shadowClip	= ubo.lights[i].viewMatrix * vec4(fragpos, 1.0);

		float shadowFactor;
		#ifdef USE_PCF
			shadowFactor= filterPCF(shadowClip, i);
		#else
			shadowFactor = textureProj(shadowClip, i, vec2(0.0));
		#endif

		fragcolor *= shadowFactor;
	}
	return fragcolor;
}

void main()
{
	// Get G-Buffer values
	vec3 fragPos = texture(samplerposition, inUV).rgb;
	vec3 normal = texture(samplerNormal, inUV).rgb;
	vec4 albedo = texture(samplerAlbedo, inUV);
	
	// Debug display
	if (ubo.displayDebugTarget > 0) {
		switch (ubo.displayDebugTarget) {
			case 1:
				outFragcolor.rgb = fragPos;
				break;
			case 2:
				outFragcolor.rgb = normal.rgb;
				break;
			case 3:
				outFragcolor.rgb = albedo.rgb;
				break;
			case 4:
				outFragcolor.rgb = albedo.aaa;
				break;
		}
		outFragcolor.a = 1.0;
		return;
	}
	
	// Ambient part
	vec3 fragcolor  = albedo.rgb * AMBIENT_LIGHT;

	vec3 N = normalize(normal);
	
	for(int i = 0; i < LIGHT_COUNT; ++i)
	{
		// Vector to light
		vec3 L = ubo.lights[i].position.xyz - fragPos;
		// Distance from light to fragment position
		float dist = length(L);
		L = normalize(L);

		// Viewer to fragment
		vec3 V = PushConstants.pos.xyz - fragPos;
		V = normalize(V);
		
		//if(dist < ubo.lights[i].radius)
		{
			// Light to fragment
			L = normalize(L);

			// Attenuation
			float atten = ubo.lights[i].radius / (pow(dist, 2.0) + 1.0);

			// Diffuse part
			vec3 N = normalize(normal.xyz);
			float NdotL = max(0.0, dot(N, L));
			vec3 diff = ubo.lights[i].color.xyz * albedo.rgb * NdotL * atten;

			// Specular part
			// Specular map values are stored in alpha of albedo mrt
			vec3 R = reflect(-L, N);
			float NdotR = max(0.0, dot(R, V));
			vec3 spec = ubo.lights[i].color.xyz * albedo.a * pow(NdotR, 16.0) * atten;

			fragcolor += diff + spec;
		}
	}
	#ifdef USE_SHADOWS
		fragcolor = shadow(fragcolor, fragPos);
	#endif
	outFragcolor = vec4(fragcolor, 1.0);
}