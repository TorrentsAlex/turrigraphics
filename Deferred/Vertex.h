#ifndef TURRI_VERTEX
#define TURRI_VERTEX

//Third-party libraries
#include <GL/glew.h>			//The OpenGL Extension Wrangler
#include <glm/glm.hpp>			//OpenGL Mathematics

struct Position {
	GLfloat  x;
	GLfloat  y;
	GLfloat  z;

	glm::vec3 toVec3() {
		return glm::vec3(x, y, z);
	}
};

struct NormalVector {
	GLfloat  nx;
	GLfloat  ny;
	GLfloat  nz;
};

struct UV {
	GLfloat u;
	GLfloat v;
	glm::vec2 toVec2() {
		return glm::vec2(u, v);
	}
};

struct Vertex {
	Position position;
	NormalVector normal;
	UV uv;
	Position tangent;
	Position bitangent;

	void setPosition(GLfloat  x, GLfloat  y, GLfloat  z) {
		position.x = x;
		position.y = y;
		position.z = z;
	}
	
	void setTangent(GLfloat  x, GLfloat  y, GLfloat  z) {
		tangent.x = x;
		tangent.y = y;
		tangent.z = z;
	}
	
	void setBitangent(GLfloat  x, GLfloat  y, GLfloat  z) {
		bitangent.x = x;
		bitangent.y = y;
		bitangent.z = z;
	}

	void setUV(float u, float v) {
		uv.u = u;
		uv.v = v;
	}

	void setNormal(GLfloat  x, GLfloat  y, GLfloat  z) {
		normal.nx = x;
		normal.ny = y;
		normal.nz = z;
	}
};

struct ObjectTransformation {
	glm::vec3 position;
	glm::vec3 scale;
	glm::vec3 rotation;
	float angle;
};

#endif
