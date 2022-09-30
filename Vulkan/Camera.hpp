#pragma once

class Camera {
	glm::vec3 Up{};
public:

	glm::vec3 Pos{};
	glm::vec3 Ang{};
	glm::vec3 front{};
	glm::vec3 right{};

	glm::mat4 View{};

	glm::mat4 View_Proj{};

	glm::vec3 Offset{};

	CameraPushConstant CPC;

	CameraUniformBuffer CamUBO;						//	Doesnt Need Cleanup
	std::vector<VkBuffer> uboCamBuff = {};			//	Cleaned Up
	std::vector<VmaAllocation> uboCamAlloc = {};	//	Cleaned Up

public:

	Camera() : Pos(glm::vec3(0, 0, 0)), Ang(glm::vec3(0, 0, -1)), Up(glm::vec3(0.0f, 1.0f, 0.0f)) {
		View = glm::lookAt(Pos, Pos + Ang, Up);
		Offset = glm::vec3(0, 2, 0);
		//
		const size_t SwapChainCount = WorldEngine::VulkanDriver::swapChain.images.size();
		uboCamBuff.resize(SwapChainCount);
		uboCamAlloc.resize(SwapChainCount);
		for (size_t i = 0; i < SwapChainCount; i++)
		{
			//
			//	Uniform Buffer Creation
			VkBufferCreateInfo uniformBufferInfo = vks::initializers::bufferCreateInfo(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(DShadow));
			uniformBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			VmaAllocationCreateInfo uniformAllocInfo = {};
			uniformAllocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
			uniformAllocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
			vmaCreateBuffer(WorldEngine::VulkanDriver::allocator, &uniformBufferInfo, &uniformAllocInfo, &uboCamBuff[i], &uboCamAlloc[i], nullptr);
		}
	}

	~Camera()
	{
		//
		for (int i = 0; i < uboCamBuff.size(); i++)
		{
			vmaDestroyBuffer(WorldEngine::VulkanDriver::allocator, uboCamBuff[i], uboCamAlloc[i]);
		}
	}

	void GoForward(float Speed) {
		Pos += Speed * Ang;
		View = glm::lookAt(Pos, Pos + Ang, Up);
	}
	void GoBackward(float Speed) {
		Pos -= Speed * Ang;
		View = glm::lookAt(Pos, Pos + Ang, Up);
	}
	void GoLeft(float Speed) {
		Pos -= glm::normalize(glm::cross(Ang, Up)) * Speed;
		View = glm::lookAt(Pos, Pos + Ang, Up);
	}
	void GoRight(float Speed) {
		Pos += glm::normalize(glm::cross(Ang, Up)) * Speed;
		View = glm::lookAt(Pos, Pos + Ang, Up);
	}

	void SetPosition(const glm::vec3& NewPosition) {
		Pos = NewPosition;
		View = glm::lookAt(Pos, Pos + Ang, Up);
	}

	void SetAngle(const glm::vec3& NewAngle) {
		Ang = NewAngle;
		View = glm::lookAt(Pos, Pos + Ang, Up);
	}
	bool firstMouse = true;
	double lastX = 0.f;
	double lastY = 0.f;
	float yaw = 0.f;
	float pitch = 0.f;
	float sensitivity = 0.15f;

	void DoLook(const double& deltaX, const double& deltaY)
	{

		yaw += deltaX * sensitivity;
		pitch += deltaY * sensitivity;

		yaw -= 360.0f * std::floor((yaw + 180.0f) * (1.0f / 360.0f));

		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;

		front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		front.y = sin(glm::radians(pitch));
		front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

		right = -glm::vec3(front.z, front.y, -front.x);

		Ang = glm::normalize(front);
		View = glm::lookAt(Pos, Pos + Ang, Up);
	}

	const float getYaw() const {
		return yaw;
	}
	const glm::vec3 getOffset() const {
		return Offset;
	}

	//
	//	TODO: Only update the matrices when the camera actually moves/rotates
	const CameraPushConstant& GetCPC(const float& ScrWidth, const float& ScrHeight, const float& zNear, const float& zFar, const float& FOV)
	{
		CPC.view_proj = glm::perspective(glm::radians(FOV), ScrWidth/ScrHeight, zNear, zFar);
		CPC.view_proj[1][1] *= -1;

		CPC.view_proj *= View;
		View_Proj = CPC.view_proj;

		CPC.pos = glm::vec4(Pos.x, Pos.y, Pos.z, 1.f);
		return CPC;
	}

	void UpdateCameraUBO(size_t CurFrame, const float& ScrWidth, const float& ScrHeight, const float& zNear, const float& zFar, const float& FOV)
	{
		CamUBO.view_proj = glm::perspective(glm::radians(FOV), ScrWidth / ScrHeight, zNear, zFar);
		CamUBO.view_proj[1][1] *= -1;

		CamUBO.view_proj *= View;
		View_Proj = CPC.view_proj;
		memcpy(uboCamAlloc[CurFrame]->GetMappedData(), &CamUBO, sizeof(CameraUniformBuffer));
	}

	void DrawGUI()
	{
	}
};

namespace WorldEngine
{
	namespace SceneGraph
	{
		//
		//	Empty namespace to hide variables
		namespace
		{
			Camera* _Camera;
		}

		Camera* GetCamera() {
			return _Camera;
		}
	}
}