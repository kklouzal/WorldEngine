#version 450

layout(location = 0) in vec4 inPos;
layout(location = 1) in mat4 inMat; //  location 1,2,3,4

layout(location = 0) out mat4 outMat; //  location 0,1,2,3

void main()
{
	outMat = inMat;
	gl_Position = inPos;
}