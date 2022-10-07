#version 450

layout(location = 0) in vec4 inPosition;
//layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in vec3 inTangent;
layout(location = 5) in vec4 inBones;
layout(location = 6) in vec4 inWeights;
//
layout(location = 7) in mat4 inInstanceMat; //  location 7,8,9,10

layout (binding = 0) uniform UBO {
    mat4 view_proj;
} ubo;

layout(std140, binding = 1) readonly buffer InstanceData_Animated {
    mat4 Matrices[32];
} Joint[];

layout(std140, binding = 2) readonly buffer InverseBindPoses {
    mat4 joint[];
} ibp;

layout(location = 0) out vec3 outNormal;
layout(location = 1) out vec2 outUV;
//layout(location = 2) out vec3 outColor;
layout(location = 3) out vec4 outWorldPos;
layout(location = 4) out vec3 outTangent;

void main() {
    outWorldPos = inInstanceMat * inPosition;
    outUV = inTexCoord;
    //outColor = inColor;

    mat4 skinMat =
        inWeights.x * Joint[0].Matrices[int(inBones.x)] * ibp.joint[int(inBones.x)] +
        inWeights.y * Joint[0].Matrices[int(inBones.y)] * ibp.joint[int(inBones.y)] +
        inWeights.z * Joint[0].Matrices[int(inBones.z)] * ibp.joint[int(inBones.z)] +
        inWeights.w * Joint[0].Matrices[int(inBones.w)] * ibp.joint[int(inBones.w)];

    //gl_Position = ubo.view_proj * ssbo.model[gl_InstanceIndex] * inPosition;
    gl_Position = ubo.view_proj * inInstanceMat * skinMat * inPosition;
    
    
    mat3 mNormal = transpose(inverse(mat3(inInstanceMat)));
    outNormal = mNormal * normalize(inNormal);
    outTangent = mNormal * normalize(inTangent);

}