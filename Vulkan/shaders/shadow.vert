#version 450

layout(location = 0) in vec4 inPos;
//
layout(location = 1) in vec4 inMatX; //  location 1,2,3,4
layout(location = 2) in vec4 inMatY; //  location 1,2,3,4
layout(location = 3) in vec4 inMatZ; //  location 1,2,3,4
layout(location = 4) in vec4 inMatW; //  location 1,2,3,4

layout(location = 0) out vec4 outMatX;
layout(location = 1) out vec4 outMatY;
layout(location = 2) out vec4 outMatZ;
layout(location = 3) out vec4 outMatW;

void main()
{
	//outInstanceMat = inInstanceMat;
	outMatX = inMatX;
	outMatY = inMatY;
	outMatZ = inMatZ;
	outMatW = inMatW;
	gl_Position = inPos;
}