#version 450

#define LIGHT_COUNT 6
#define SHADOW_FACTOR 0.25
#define AMBIENT_LIGHT 0.25

layout (input_attachment_index = 0, binding = 0) uniform subpassInput samplerposition;
layout (input_attachment_index = 1, binding = 1) uniform subpassInput samplerNormal;
layout (input_attachment_index = 2, binding = 2) uniform subpassInput samplerAlbedo;
layout(binding = 3) uniform sampler2DArray samplerShadowMap;

layout(location = 0) in vec2 inUV;

layout(location = 0) out vec4 outFragcolor;

struct DLight {
	vec4 position;
	vec4 target;
	vec4 color;
	mat4 viewMatrix;
};

layout(std140, binding = 4) uniform DComposition
{
	DLight lights[6];
	int displayDebugTarget;
	vec4 camPos;
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

		fragcolor *= filterPCF(shadowClip, i);
	}
	return fragcolor;
}

void main()
{
	// Get G-Buffer values
//	vec3 fragPos = texture(samplerposition, inUV).rgb;
//	vec3 normal = texture(samplerNormal, inUV).rgb;
//	vec4 albedo = texture(samplerAlbedo, inUV);
	vec3 fragPos = subpassLoad(samplerposition).rgb;
	vec3 normal = subpassLoad(samplerNormal).rgb;
	vec4 albedo = subpassLoad(samplerAlbedo);
	
	// Debug display
	if (ubo.displayDebugTarget > 0) {
		switch (ubo.displayDebugTarget) {
			case 1:	//F2
				outFragcolor.rgb = shadow(vec3(1.0), fragPos).rgb;
				break;
			case 2: //F3
				outFragcolor.rgb = fragPos;
				break;
			case 3: //F4
				outFragcolor.rgb = normal;
				break;
			case 4: //F5
				outFragcolor.rgb = albedo.rgb;
				break;
			case 5: //F6
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
		vec3 V = ubo.camPos.xyz - fragPos;
		V = normalize(V);

		float lightCosInnerAngle = cos(radians(15.0));
		float lightCosOuterAngle = cos(radians(25.0));
		float lightRange = 100.0;

		// Direction vector from source to target
		vec3 dir = normalize(ubo.lights[i].position.xyz - ubo.lights[i].target.xyz);

		// Dual cone spot light with smooth transition between inner and outer angle
		float cosDir = dot(L, dir);
		float spotEffect = smoothstep(lightCosOuterAngle, lightCosInnerAngle, cosDir);
		float heightAttenuation = smoothstep(lightRange, 0.0f, dist);

		// Diffuse lighting
		float NdotL = max(0.0, dot(N, L));
		vec3 diff = vec3(NdotL);

		// Specular lighting
		vec3 R = reflect(-L, N);
		float NdotR = max(0.0, dot(R, V));
		vec3 spec = vec3(pow(NdotR, 16.0) * albedo.a * 2.5);

		fragcolor += vec3((diff + spec) * spotEffect * heightAttenuation) * ubo.lights[i].color.rgb * albedo.rgb;
	}
	outFragcolor = vec4(shadow(fragcolor, fragPos), 1.0);
}