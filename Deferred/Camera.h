

#include <iostream>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <glm/glm.hpp>			//OpenGL Mathematics
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

enum class CameraMove{ none, forward, backwards, left, right,  up, down};

class Camera {
public:
	
	float cAspectRatio;
	glm::mat4 cProjectionMatrix;
	glm::mat4 cViewMatrix;
	float cFOV;
	float cFar;
	float cNear;
	float cProjectionWidth;
	float cProjectionHeight;

	glm::vec2 cScreenSize;

	glm::vec3 cCameraPos;
	glm::vec3 cCameraFront;
	glm::vec3 cCameraUp;
	glm::vec3 cCameraRight;

	float cAngle;

	glm::vec2 rotation;
	glm::vec2 prevMouse;
	float yaw = 0.0f;
	float pitch;
	float velocity;

	float up = 1.0f;

	bool firstEntry = true;

	Camera();
	Camera(Camera& c);
	~Camera();
	void initializeZBuffer(glm::vec2 windowResolution);
	void setPerspectiveCamera();
	void setAngle(float angle);

	void setViewMatrix();
	void setCameraPosition(glm::vec3 pos);
	void setCameraFront(glm::vec3 front);
	void calcCameraFront();
	float getAngle();

	float getFar();
	float getNear();

	void moveTo(CameraMove cm);
	void rotate(glm::vec2 mousePos);

	glm::vec3 getPosition();
	glm::mat4 getProjectionCamera();
	glm::mat4 getViewMatrix();

	void addCameraVelocity(float v);

	void resetFirstEntry();
};
