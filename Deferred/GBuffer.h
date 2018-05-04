#ifndef TURRI_GBUFFER
#define TURRI_GBUFFER
#include <Windows.h>

#include <GL/glew.h>

#include <string>
#include <vector>
#include <iostream>
#include <fstream>

#include "Vertex.h"
#include "Entity.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp> //OpenGL Mathematics
#include <glm/gtc/matrix_transform.hpp>

struct Shader {
	GLuint programID;
	GLint vertID;
	GLint fragID;
	std::string vertPath;
	std::string fragPath;
	int numAttributes = 0;
};

using namespace std;

class GBuffer {
public:

	static Shader createShader(std::string pathVS, std::string pathFS);
	static void linkShaders(Shader shader);

	static void addAttribute(Shader shader, std::string attributeName);

	static void use(Shader shader);
	static void unuse(Shader shader);

	static void initFrameBuffer(GLuint *gbuffer);
	static void genTexture(GLuint *texture, int attachment, glm::vec2 size);
	static void genDepthTexture(GLuint *texture, int attachment, glm::vec2 size);
	static void closeGBufferAndDepth(int drawSize, GLuint attachments[], GLuint *depth, glm::vec2 size);

	static void sendCubemap(Shader shader, std::string name, GLuint texture);
	static void sendTexture(Shader shader, std::string name, GLuint texture, int TEXTURE_GL, int unit);
	static void sendDataToGPU(Vertex &data, int numVertices);
	static void bindVertexArrayBindBuffer(GLuint vao, GLuint vbo);
	static void unbindVertexUnbindBuffer();

	static void sendUniform(Shader shader, std::string name, int value);
	static void sendUniform(Shader shader, std::string name, float value);

	static void sendUniform(Shader shader, std::string name, bool boolean);
	static void sendUniform(Shader shader, std::string name, glm::vec3 vec3);
	static void sendUniform(Shader shader, std::string name, glm::mat4 value);
	static void sendUniform(Shader shader, std::string name, glm::mat3 value);
	static void sendUniform(Shader shader, std::string name, glm::vec2 value);

	static void unbindTextures();
private:
	GBuffer() = delete;
	~GBuffer() = delete;

	static void compileShader(GLenum shaderType, const std::string &shaderFileName);
};
#endif
