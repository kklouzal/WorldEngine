#pragma once

class Camera {
	glm::vec3 Up{};
public:

	glm::vec3 Pos{};
	glm::vec3 Ang{};
	glm::vec3 front{};

	glm::mat4 View{};

	glm::vec3 Offset{};

public:

	Camera() : Pos(glm::vec3(0, 0, 0)), Ang(glm::vec3(0, 0, -1)), Up(glm::vec3(0.0f, 1.0f, 0.0f)) {
		View = glm::lookAt(Pos, Pos + Ang, Up);
		Offset = glm::vec3(0, 3, 0);
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
	void DoLook(double deltaX, double deltaY) {

		float sensitivity = 0.15;
		deltaX *= sensitivity;
		deltaY *= sensitivity;

		yaw += deltaX;
		pitch += deltaY;

		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;

		front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		front.y = sin(glm::radians(pitch));
		front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		Ang = glm::normalize(front);
		View = glm::lookAt(Pos, Pos + Ang, Up);
	}

	float getYaw() {
		return yaw;
	}
	glm::vec3 getOffset() {
		return Offset;
	}
};