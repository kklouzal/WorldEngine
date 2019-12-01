#pragma once

class Camera {
	glm::vec3 Up{};
public:

	glm::vec3 Pos{};
	glm::vec3 Ang{};
	glm::vec3 front{};

	glm::mat4 View{};

	glm::vec3 Offset{};
	glm::vec3 front2{};
	glm::vec3 Ang2{};

public:

	Camera() : Pos(glm::vec3(0, 0, 0)), Ang(glm::vec3(0, 0, -1)), Up(glm::vec3(0.0f, 1.0f, 0.0f)) {
		View = glm::lookAt(Pos, Pos + Ang, Up);
		Offset = glm::vec3(0, 3, 0);
	}

	glm::vec3 GetForward(float Speed) {
		return Pos + Speed * glm::vec3(Ang2.x, 0, Ang2.z);
	}
	glm::vec3 GetBackward(float Speed) {
		return Pos - Speed * glm::vec3(Ang2.x, 0, Ang2.z);
	}
	glm::vec3 GetLeft(float Speed) {
		return Pos - glm::normalize(glm::cross(glm::vec3(Ang2.x, 0, Ang2.z), Up)) * Speed;
	}
	glm::vec3 GetRight(float Speed) {
		return Pos + glm::normalize(glm::cross(glm::vec3(Ang2.x, 0, Ang2.z), Up)) * Speed;
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
	float pitch2 = 0;
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



		if (pitch2 > 44.0f)
			pitch2 = 44.0f;
		if (pitch2 < -44.0f)
			pitch2 = -44.0f;

		front2.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch2));
		front2.y = sin(glm::radians(pitch2));
		front2.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch2));
		Ang2 = glm::normalize(front2);
	}

	float getYaw() {
		return yaw;
	}
	glm::vec3 getOffset() {
		return Offset;
	}
};