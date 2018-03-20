#include "Camera.h"

Camera::Camera():
	cAspectRatio(1),
	cFOV(65.0f),
	cFar(2001.0f),
	cNear(0.1f),
	cProjectionWidth(15.0f),
	cProjectionHeight(15.0f),
	cCameraPos(60.0f, 59.0f, 108.0f),
	cCameraFront(-1.0f, 0.0f, -1.0f),
	cCameraUp(0.0f, 1.0f, 0.0f),
	cCameraRight(0.0f, 0.0f, 1.0f),
	prevMouse(0.0f),
	velocity(0.05f) {
	cCameraRight = glm::normalize(glm::cross(cCameraUp, cCameraFront));
}

Camera::Camera(Camera& c) {
	cAspectRatio = c.cAspectRatio;
	cFOV = c.cFOV;
	cFar = c.cFar;
	cNear = c.cNear;
	cProjectionWidth = c.cProjectionWidth;
	cProjectionHeight = c.cProjectionHeight;
	cCameraPos = c.cCameraPos;
	cCameraFront = c.cCameraFront;
	cCameraUp = c.cCameraUp;
}

Camera::~Camera() {}

void Camera::initializeZBuffer(glm::vec2 windowResolution) {
	//Initialize the Zbuffer
	glEnable(GL_DEPTH_TEST);

	glViewport(0, 0, windowResolution.x, windowResolution.y);
	cAspectRatio = windowResolution.x / windowResolution.y;
	cScreenSize = windowResolution;
}

// Setters
void Camera::setPerspectiveCamera() {
	cProjectionMatrix = glm::perspective(cFOV, cAspectRatio, cNear, cFar);
}

void Camera::setViewMatrix() {
	glm::vec3 cameraDirection = glm::normalize(-cCameraFront);
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	cCameraRight = glm::normalize(glm::cross(up, cameraDirection));
	cCameraUp = glm::normalize(glm::cross(cameraDirection, cCameraRight));
	cViewMatrix = glm::lookAt(cCameraPos, cCameraPos + cCameraFront, cCameraUp);
}

void Camera::setCameraPosition(glm::vec3 pos) {
	cCameraPos = pos;
	setViewMatrix();
}

void Camera::setCameraFront(glm::vec3 front) {
	//cCameraFront = glm::vec3(front.x, front.y, front.z + 1.0);
}

// Getters
glm::mat4 Camera::getProjectionCamera() {
	return cProjectionMatrix;
}

glm::mat4 Camera::getViewMatrix() {
	return cViewMatrix;
}

glm::vec3 Camera::getPosition() {
	return cCameraPos;
}

void Camera::setAngle(float angle) {
	cAngle = angle;
}

float Camera::getAngle() {
	return cAngle;
}

float Camera::getFar() {
	return cFar;
}

float Camera::getNear() {
	return cNear;
}

void Camera::rotate(glm::vec2 mousePos) {
	if (firstEntry) {
		prevMouse = mousePos;
		firstEntry = false;
	}

	glm::vec2 offset = glm::vec2(mousePos.x - prevMouse.x, prevMouse.y - mousePos.y);
	prevMouse = mousePos;
	float sensitivity = 0.1f;
	offset *= sensitivity;

	yaw += offset.x;
	pitch += offset.y;

	if (pitch > 89.0f) {
		pitch = 89.0f;
	}
	if (pitch < -89.0f) {
		pitch = -89.0f;
	}

	glm::vec3 front;
	front.x = glm::cos(glm::radians(yaw)) * glm::cos(glm::radians(pitch));
	front.y = glm::sin(glm::radians(pitch));
	front.z = glm::sin(glm::radians(yaw)) * glm::cos(glm::radians(pitch));

	cCameraFront = front;
	setViewMatrix();
}

void Camera::resetFirstEntry() {
	firstEntry = true;
}

void Camera::moveTo(CameraMove cm) {
	switch (cm) {
		case  CameraMove::forward:
			setCameraPosition(cCameraPos += glm::normalize(cCameraFront)* velocity);
		break;
		case  CameraMove::backwards:
			setCameraPosition(cCameraPos -= glm::normalize(cCameraFront)* velocity);
		break;
		case  CameraMove::left:
			cCameraPos -= glm::normalize(glm::cross(cCameraFront, cCameraUp)) * velocity;
			setCameraPosition(cCameraPos);
		break;
		case  CameraMove::right:
			cCameraPos += glm::normalize(glm::cross(cCameraFront, cCameraUp)) * velocity;
			setCameraPosition(cCameraPos);
		break;
		case  CameraMove::up:
			cCameraPos += cCameraUp * velocity;
			setCameraPosition(cCameraPos);
		break;
		case  CameraMove::down:
			cCameraPos -= cCameraUp * velocity;
			setCameraPosition(cCameraPos);
		break;
		case CameraMove::none:
		default:
		break;
	}
}

void Camera::addCameraVelocity(float v) { velocity += v;}