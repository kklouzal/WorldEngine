#version 450

layout(location = 0) in vec4 inPosition;
//layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in vec3 inTangent;

layout(std140, binding = 0) readonly buffer InstanceData {
    mat4 model[];
} ssbo;

layout (binding = 1) uniform UBO {
    mat4 view_proj;
} ubo;

layout(location = 0) out vec3 outNormal;
layout(location = 1) out vec2 outUV;
//layout(location = 2) out vec3 outColor;
layout(location = 3) out vec4 outWorldPos;
layout(location = 4) out vec3 outTangent;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main() {
    outWorldPos = ssbo.model[gl_InstanceIndex] * inPosition;
    outUV = inTexCoord;
    //outColor = inColor;

    gl_Position = ubo.view_proj * ssbo.model[gl_InstanceIndex] * inPosition;
    
	mat3 mNormal = transpose(inverse(mat3(ssbo.model[gl_InstanceIndex])));
	outNormal = mNormal * normalize(inNormal);
    outTangent = mNormal * normalize(inTangent);
}