#pragma once

class Camera {
	glm::vec3 Up{};
public:

	glm::vec3 Pos{};
	glm::vec3 Ang{};
	glm::vec3 front{};
	glm::vec3 right{};

	glm::mat4 View{};

	glm::vec3 Offset{};

	CameraPushConstant CPC;

public:

	Camera() : Pos(glm::vec3(0, 0, 0)), Ang(glm::vec3(0, 0, -1)), Up(glm::vec3(0.0f, 1.0f, 0.0f)) {
		View = glm::lookAt(Pos, Pos + Ang, Up);
		Offset = glm::vec3(0, 1, 0);
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
	double lastX = 0;
	double lastY = 0;
	float yaw = 0;
	float pitch = 0;
	float sensitivity = 0.15;

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
		//ubo.view = glm::lookAt(glm::vec3(512.0f, 512.0f, 128.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		CPC.view = View;
		CPC.proj = glm::perspective(glm::radians(FOV), ScrWidth/ScrHeight, zNear, zFar);
		CPC.proj[1][1] *= -1;
		return CPC;
	}
};