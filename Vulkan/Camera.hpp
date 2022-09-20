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

public:

	Camera() : Pos(glm::vec3(0, 0, 0)), Ang(glm::vec3(0, 0, -1)), Up(glm::vec3(0.0f, 1.0f, 0.0f)) {
		View = glm::lookAt(Pos, Pos + Ang, Up);
		Offset = glm::vec3(0, 2, 0);
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

	void DrawGUI()
	{
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
		ImGui::SetNextWindowPos(ImVec2(WorldEngine::VulkanDriver::WIDTH / 2 - 15, WorldEngine::VulkanDriver::HEIGHT / 2 - 15));
		ImGui::SetNextWindowBgAlpha(0.f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::Begin("Example: Simple overlay", 0, window_flags);
		ImGui::Image(WorldEngine::GUI::UseTextureFile("media/crosshairs/focus1.png"), ImVec2(30, 30), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), ImVec4(0, 0, 0, 0));
		ImGui::PopStyleVar();
		ImGui::End();
	}
};