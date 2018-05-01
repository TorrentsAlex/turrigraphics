#include "GBuffer.h"

void GBuffer::compileShader(GLenum shaderType, const string &shaderFileName) {

	//Open, read, send to openGl and compile the GLSL source code for the vertex shader
	//Open the file
	ifstream shaderFile(shaderFileName);
	if (shaderFile.fail()) {
		//Get additional information related to the error
		perror(shaderFileName.c_str());
		std::cout <<"OpenGL.cpp:188 error: " << shaderFileName << " Failed to open " << std::endl;
	}

	//File contents stores all the text in the file
	string fileContents = "";
	//line is used to grab each line of the file
	string line;
	//Get all the lines in the file and add it to the contents
	while (getline(shaderFile, line)) {
		fileContents += line + "\n";
	}
	//Close the file
	shaderFile.close();

	const char *contentsPtr = fileContents.c_str();
	//tell opengl that we want to use fileContents as the contents of the shader file
	glShaderSource(shaderType, 1, &contentsPtr, nullptr);
	//Compile the shader
	glCompileShader(shaderType);

	//Check for errors
	GLint isCompiled = 0;
	glGetShaderiv(shaderType, GL_COMPILE_STATUS, &isCompiled);

	if (isCompiled == GL_FALSE) {
		//Get the error message. First we get the length and next we read the content
		GLint maxLength = 0;
		glGetShaderiv(shaderType, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		std::vector<GLchar> errorLog(maxLength);
		glGetShaderInfoLog(shaderType, maxLength, &maxLength, &errorLog[0]);

		// Don't leak the shader.
		glDeleteShader(shaderType);

		// Exit with failure.
		printf("%s\n", &(errorLog[0]));
		std::cout <<"GBuffer.cpp:49 error: " << shaderFileName << " failed to compile " << std::endl;
	}
	// Shader compilation is successful.
}

Shader GBuffer::createShader(std::string pathVS, std::string pathFS) {
	Shader sh;
	sh.programID = glCreateProgram();

	sh.vertID = glCreateShader(GL_VERTEX_SHADER);		
	
	if (sh.vertID == 0) {
		std::cout << "GBuffer.cpp:66 error: " << pathVS << " failed to be created!" << std::endl;
	}

	sh.fragID = glCreateShader(GL_FRAGMENT_SHADER);
	if (sh.fragID == 0) {
		std::cout << "GBuffer.cpp:66 error: " << pathFS << " failed to be created!" << std::endl;
	}
	sh.vertPath = pathVS;
	sh.fragPath = pathFS;

	compileShader(sh.vertID, sh.vertPath);
	compileShader(sh.fragID, sh.fragPath);
	return sh;
}

void GBuffer::linkShaders(Shader shader) {

	//Attach our shaders to our program
	glAttachShader(shader.programID, shader.vertID);
	glAttachShader(shader.programID, shader.fragID);

	//Link our program
	glLinkProgram(shader.programID);

	//Note the different functions here: glGetProgram* instead of glGetShader*.
	GLint isLinked = 0;
	glGetProgramiv(shader.programID, GL_LINK_STATUS, (int *)&isLinked);
	if (isLinked == GL_FALSE) {
		GLint maxLength = 0;
		glGetProgramiv(shader.programID, GL_INFO_LOG_LENGTH, &maxLength);

		//The maxLength includes the NULL character
		std::vector<char> errorLog(maxLength);
		glGetProgramInfoLog(shader.programID, maxLength, &maxLength, &errorLog[0]);

		//We don't need the program anymore.
		glDeleteProgram(shader.programID);

		glDeleteShader(shader.vertID);
		glDeleteShader(shader.fragID);


		// Exit with failure.
		printf("%s\n", &(errorLog[0]));
		std::cout <<"OpenGL.cpp:271 error: " << "Shaders failed to link" << std::endl;
	}

	//Always detach shaders after a successful link.
	glDetachShader(shader.programID, shader.vertID);
	glDeleteShader(shader.vertID);

	glDetachShader(shader.programID, shader.fragID);
	glDeleteShader(shader.fragID);
}

void GBuffer::addAttribute(Shader shader, string attributeName) {
	glBindAttribLocation(shader.programID, shader.numAttributes, attributeName.c_str());
	shader.numAttributes++;
}

void GBuffer::use(Shader shader) {
	glUseProgram(shader.programID);
	for (int i = 0; i< shader.numAttributes; i++) {
		glEnableVertexAttribArray(i);
	}
}

void GBuffer::unuse(Shader shader) {
 	for (int i = 0; i< shader.numAttributes; i++) {
		glDisableVertexAttribArray(i);
	}
}

void GBuffer::initFrameBuffer(GLuint *buffer) {
	glGenFramebuffers(1, buffer);
	glBindFramebuffer(GL_FRAMEBUFFER, *buffer);
}

void GBuffer::genTexture(GLuint *texture, int attachment, glm::vec2 size) {

	glGenTextures(1, texture);
	glBindTexture(GL_TEXTURE_2D, *texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, size.x, size.y, 0, GL_RGBA, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, *texture, 0);
}

void GBuffer::genDepthTexture(GLuint *texture, int attachment, glm::vec2 size) {

	glGenTextures(1, texture);
	glBindTexture(GL_TEXTURE_2D, *texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, size.x, size.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, *texture, 0);
	glDrawBuffer(GL_NONE);

	/* GLuint FramebufferName = 0;
 glGenFramebuffers(1, &FramebufferName);
 glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);

 // Depth texture. Slower than a depth buffer, but you can sample it later in your shader
 GLuint depthTexture;
 glGenTextures(1, &depthTexture);
 glBindTexture(GL_TEXTURE_2D, depthTexture);
 glTexImage2D(GL_TEXTURE_2D, 0,GL_DEPTH_COMPONENT16, 1024, 1024, 0,GL_DEPTH_COMPONENT, GL_FLOAT, 0);
 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

 glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTexture, 0);

 glDrawBuffer(GL_NONE); // No color buffer is drawn to.*/
}

void GBuffer::closeGBufferAndDepth(int drawSize ,GLuint attachments[], GLuint *depth, glm::vec2 size) {
	glDrawBuffers(drawSize, attachments);

	// Depth buffer
	glGenRenderbuffers(1, depth);
	glBindRenderbuffer(GL_RENDERBUFFER, *depth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, size.x, size.y);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, *depth);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "Framebuffer not complete!" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GBuffer::sendDataToGPU(Vertex* data, int numVertices) {
	// Put all the vertices into the VBO
	glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(Vertex), data, GL_STATIC_DRAW);

	//Draw a set of elements(numVertices) from the VBO as GL_TRIANGLES. The first vertex is in the 0th position
	glDrawArrays(GL_TRIANGLES, 0, numVertices);

}

void GBuffer::bindVertexArrayBindBuffer(GLuint vao, GLuint vbo) {
	glBindVertexArray(vao);

	//Bind the vertex buffer object
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
}

void GBuffer::unbindVertexUnbindBuffer() {

	//Unbind the VBO and VAO
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

/* Shader */
void GBuffer::sendUniform(Shader shader, std::string name, int value) {
	glUniform1i(glGetUniformLocation(shader.programID, name.c_str()), value);
}

void GBuffer::sendUniform(Shader shader, std::string name, float value) {
	glUniform1f(glGetUniformLocation(shader.programID, name.c_str()), value);
}

void GBuffer::sendUniform(Shader shader, std::string name, bool boolean) {
	glUniform1i(glGetUniformLocation(shader.programID, name.c_str()), boolean ? 1 : 0);
}

void GBuffer::sendUniform(Shader shader, std::string name, glm::vec3 value) {
	glUniform3fv(glGetUniformLocation(shader.programID, name.c_str()), 1, glm::value_ptr(value));
}

void GBuffer::sendUniform(Shader shader, std::string name, glm::mat4 value) {
	glUniformMatrix4fv(glGetUniformLocation(shader.programID, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}

void GBuffer::sendUniform(Shader shader, std::string name, glm::mat3 value) {
	glUniformMatrix3fv(glGetUniformLocation(shader.programID, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}

void GBuffer::sendUniform(Shader shader, std::string name, glm::vec2 value) {
	glUniform2f(glGetUniformLocation(shader.programID, name.c_str()), value.x, value.y);
}


void GBuffer::sendCubemap(Shader shader, std::string name, GLuint texture) {
	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
}

void GBuffer::sendTexture(Shader shader, std::string name, GLuint texture, int TEXTURE_GL, int unit) {

	GLint location = glGetUniformLocation(shader.programID, name.c_str());
	glUniform1i(location, unit);
	glActiveTexture(TEXTURE_GL);
	glBindTexture(GL_TEXTURE_2D, texture);
}

void GBuffer::unbindTextures() {
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 1);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, 2);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, 3);
}
