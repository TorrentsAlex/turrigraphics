#include <windows.h>

#include <gl/glew.h>
#include <iostream>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl_gl3.h>
//
#include "Camera.h"
#include "Window.h"
#include "FPSLimiter.h"
#include "Scene.h"
#include "SceneCreator.h"
#include "InputManager.h"
#include "TextureManager.h"
#include "GBuffer.h"
#include "profiler.hpp"


#define IM_ARRAYSIZE(_ARR)  ((int)(sizeof(_ARR)/sizeof(*_ARR)))

//const glm::vec2 screenSize = {960, 540};
const glm::vec2 screenSize = { 1920, 1080};

const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
Utilities::Profiler profiler;

Camera camera;
FPSLimiter fps = FPSLimiter(true, 60, false);
Window window;

Shader shaderGBuffer;
Shader shaderPBR;
Shader shaderBlur;
Shader shaderFinal;
Shader shaderSkybox;
Shader shaderShadow;

GLuint debugTexture;
bool isdebugMode;

Scene *scene;

GLuint deferredVAO , deferredVBO;
GLuint gBVAO, gBVBO;
GLuint skyboxVAO, skyboxVBO;
GLuint frameBuffer;
GLuint reflectionGBuffer, buffNOR, buffDIF, buffPOS, buffSPEC;
GLuint reflectionBuffer, buffREFRACTION;
GLuint buffNOR2, buffDIF2, buffPOS2, buffSPEC2, buffREFLECTION;
GLuint depthBuffer;

// Lighting Pass
GLuint lihgtingBuffer, buffALBEDO, buffLUMINANCE;

// Skybox
GLuint skyboxBuffer, buffSkybox;
// Final stack pass
GLuint bloomBuffer[2];
GLuint buffBLOOM[2];
GLuint shadowTexture, shadowFBO;

void moveCameraWithKeyboard();
void sendObject(Vertex *data, GameObject object, int numVertices, Shader shader);
glm::vec3 directionalColor = { 1.0, 1.0, 1.0 };

struct Quad {
	OBJ object;
	Quad() {
		object = Geometry::LoadModelFromFile("../resources/objects/quad.obj");
	}

	void draw() {
		GBuffer::bindVertexArrayBindBuffer(deferredVAO, deferredVBO);
		GBuffer::sendDataToGPU(*object.mesh, object.numVertices);
		GBuffer::unbindVertexUnbindBuffer();
	}
	
} quad;

struct Water {
	Entity water;
	GLuint waterVAO, waterVBO;
	GLuint frameBuffer, texture;
	Shader shader;

	float moveVelocity = 0.035f;
	float moveFactor = 0.0f;
	float shineDamper = 40.0f;
	float reflectivity = 10.0f;
	float waveStrength = 0.015f;
	float textureScaleFactor = 7.0f;
	float fresnelFactor = 1.0f;
	float waterNumVertexs;

	Water() {
	}
	Water(Entity w) { water = w; }

	void GUI() {
		if (ImGui::CollapsingHeader("Water")) {
			ImGui::Text("move Factor  %f", &moveFactor);
			ImGui::SliderFloat("fresnelFactor", &fresnelFactor, 1.0f, 15.0f);
			ImGui::SliderFloat("shine  ", &shineDamper, 10.0f, 100.0f);
			ImGui::SliderFloat("wave strenght ", &waveStrength, 0.00f, 0.030f);
			ImGui::SliderFloat("reflectivity ", &reflectivity, 0.2f, 15.0f);
			ImGui::SliderFloat("dudv Velocity", &moveVelocity, 0.005f, 0.5f);
			ImGui::SliderFloat("texture Scale", &textureScaleFactor, 1.0f, 15.0f);
		}
	}

	void initShaders() {

		shader = GBuffer::createShader("../resources/shaders/water.vs", "../resources/shaders/water.fs");
		// AttRibutes

		//GBuffer::addAttribute(shader, "vertexPosition");
		//GBuffer::addAttribute(shader, "vertexNormal");
		//GBuffer::addAttribute(shader, "vertexUV");
		//GBuffer::addAttribute(shader, "vertexTangent");
		//GBuffer::addAttribute(shader, "vertexBitangent");

		GBuffer::linkShaders(shader);
	}

	void init() {
		initShaders();

		glGenVertexArrays(1, &waterVAO);
		glGenBuffers(1, &waterVBO);

		//Generate the VBO if it isn't already generated
		//This is for preventing a memory leak if someone calls twice the init method
		glBindVertexArray(waterVBO);


		//Bind the buffer object. YOU MUST BIND  the buffer vertex object before binding attributes
		glBindBuffer(GL_ARRAY_BUFFER, waterVBO);

		//Connect the xyz to the "vertexPosition" attribute of the vertex shader
		glEnableVertexAttribArray(glGetAttribLocation(shader.programID, "vertexPosition"));
		glEnableVertexAttribArray(glGetAttribLocation(shader.programID, "vertexUV"));
		glEnableVertexAttribArray(glGetAttribLocation(shader.programID, "vertexNormal"));
		glEnableVertexAttribArray(glGetAttribLocation(shader.programID, "vertexTangent"));
		glEnableVertexAttribArray(glGetAttribLocation(shader.programID, "vertexBitangent"));


		glVertexAttribPointer(glGetAttribLocation(shader.programID, "vertexPosition"), 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
		glVertexAttribPointer(glGetAttribLocation(shader.programID, "vertexUV"), 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
		glVertexAttribPointer(glGetAttribLocation(shader.programID, "vertexNormal"), 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
		glVertexAttribPointer(glGetAttribLocation(shader.programID, "vertexTangent"), 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));
		glVertexAttribPointer(glGetAttribLocation(shader.programID, "vertexBitangent"), 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, bitangent));

		// unbind the VAO and VBO
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		GBuffer::initFrameBuffer(&frameBuffer);
		GBuffer::genTexture(&texture, GL_COLOR_ATTACHMENT0, screenSize);
		GLuint attachmentsV[1] = { GL_COLOR_ATTACHMENT0 };

		GBuffer::closeGBufferAndDepth(1, attachmentsV, &depthBuffer, screenSize);
	}
	
	void update() {
		
		glBindBuffer(GL_ARRAY_BUFFER, waterVBO);
		Vertex* buff = (Vertex*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
		if (buff == nullptr) return;
		for (int i = 0; i < scene->sWater.getNumVertices(); i++) {
			buff[i].position.y = 5.0f;
		}
		glUnmapBuffer(GL_ARRAY_BUFFER);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		///---------------------------------------------

	}
	
	void draw() {
		moveFactor += moveVelocity * 0.16;
		if (moveFactor > 1.0) moveFactor = 0.0f;

		glEnable(GL_DEPTH_TEST);

		GBuffer::use(this->shader);

		GBuffer::sendTexture(shader, "textureRefraction", buffALBEDO, GL_TEXTURE0, 0);
		GBuffer::sendTexture(shader, "textureReflection", buffREFLECTION, GL_TEXTURE1, 1);
		GBuffer::sendTexture(shader, "dudvMap", scene->sWater.eMaterial.roughnessMap, GL_TEXTURE2, 2);
		GBuffer::sendTexture(shader, "normalMap", scene->sWater.eMaterial.normalMap, GL_TEXTURE3, 3);

		GBuffer::sendUniform(shader, "moveFactor", moveFactor);
		GBuffer::sendUniform(shader, "fresnelFactor", fresnelFactor);
		GBuffer::sendUniform(shader, "waveStrenght", waveStrength);
		GBuffer::sendUniform(shader, "shineDamper", shineDamper);
		GBuffer::sendUniform(shader, "reflectivity", reflectivity);
		GBuffer::sendUniform(shader, "lightPosition", scene->sLights[0].lPosition);
		GBuffer::sendUniform(shader, "lightColor", directionalColor);
		GBuffer::sendUniform(shader, "textureScaleFactor", glm::vec2(textureScaleFactor));

		GBuffer::bindVertexArrayBindBuffer(waterVAO, waterVBO);

		GBuffer::sendUniform(shader, "viewMatrix", camera.getViewMatrix());
		GBuffer::sendUniform(shader, "projectionMatrix", camera.getProjectionCamera());
		GBuffer::sendUniform(shader, "viewerPosition", camera.getPosition());

		sendObject(scene->sWater.getMesh(), scene->sWater.getGameObject(), scene->sWater.getNumVertices(), shaderGBuffer);
		
		GBuffer::unbindVertexUnbindBuffer();
		GBuffer::unuse(this->shader);
		//glDisable(GL_CLIP_DISTANCE0);

		//glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	}

} water;

#pragma region INITIALIZERS


float skyboxVertices[] = {
	// positions          
	-1000.f,  1000.f, -1000.f,
	-1000.f, -1000.f, -1000.f,
	1000.f, -1000.f, -1000.f,
	1000.f, -1000.f, -1000.f,
	1000.f,  1000.f, -1000.f,
	-1000.f,  1000.f, -1000.f,

	-1000.f, -1000.f,  1000.f,
	-1000.f, -1000.f, -1000.f,
	-1000.f,  1000.f, -1000.f,
	-1000.f,  1000.f, -1000.f,
	-1000.f,  1000.f,  1000.f,
	-1000.f, -1000.f,  1000.f,

	1000.f, -1000.f, -1000.f,
	1000.f, -1000.f,  1000.f,
	1000.f,  1000.f,  1000.f,
	1000.f,  1000.f,  1000.f,
	1000.f,  1000.f, -1000.f,
	1000.f, -1000.f, -1000.f,

	-1000.f, -1000.f,  1000.f,
	-1000.f,  1000.f,  1000.f,
	1000.f,  1000.f,  1000.f,
	1000.f,  1000.f,  1000.f,
	1000.f, -1000.f,  1000.f,
	-1000.f, -1000.f,  1000.f,

	-1000.f,  1000.f, -1000.f,
	1000.f,  1000.f, -1000.f,
	1000.f,  1000.f,  1000.f,
	1000.f,  1000.f,  1000.f,
	-1000.f,  1000.f,  1000.f,
	-1000.f,  1000.f, -1000.f,

	-1000.f, -1000.f, -1000.f,
	-1000.f, -1000.f,  1000.f,
	1000.f, -1000.f, -1000.f,
	1000.f, -1000.f, -1000.f,
	-1000.f, -1000.f,  1000.f,
	1000.f, -1000.f,  1000.f
};

void initializeVAOVBO() {

	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);

	//Generate the VBO if it isn't already generated
	//This is for preventing a memory leak if someone calls twice the init method
	glBindVertexArray(skyboxVAO);

	//Bind the buffer object. YOU MUST BIND  the buffer vertex object before binding attributes
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	// unbind the VAO and VBO
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	// GBuffer VAO VBO
	{
		glGenVertexArrays(1, &gBVAO);
		glGenBuffers(1, &gBVBO);

		//Generate the VBO if it isn't already generated
		//This is for preventing a memory leak if someone calls twice the init method
		glBindVertexArray(gBVAO);

		//Bind the buffer object. YOU MUST BIND  the buffer vertex object before binding attributes
		glBindBuffer(GL_ARRAY_BUFFER, gBVBO);

		//Connect the xyz to the "vertexPosition" attribute of the vertex shader
		glEnableVertexAttribArray(glGetAttribLocation(shaderGBuffer.programID, "vertexPosition"));
		glEnableVertexAttribArray(glGetAttribLocation(shaderGBuffer.programID, "vertexUV"));
		glEnableVertexAttribArray(glGetAttribLocation(shaderGBuffer.programID, "vertexNormal"));
		glEnableVertexAttribArray(glGetAttribLocation(shaderGBuffer.programID, "vertexTangent"));
		glEnableVertexAttribArray(glGetAttribLocation(shaderGBuffer.programID, "vertexBitangent"));


		glVertexAttribPointer(glGetAttribLocation(shaderGBuffer.programID, "vertexPosition"), 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
		glVertexAttribPointer(glGetAttribLocation(shaderGBuffer.programID, "vertexUV"), 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
		glVertexAttribPointer(glGetAttribLocation(shaderGBuffer.programID, "vertexNormal"), 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
		glVertexAttribPointer(glGetAttribLocation(shaderGBuffer.programID, "vertexTangent"), 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));
		glVertexAttribPointer(glGetAttribLocation(shaderGBuffer.programID, "vertexBitangent"), 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, bitangent));

		// unbind the VAO and VBO
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

	}

	// Deferred VAO VBO
	{
		GBuffer::initFrameBuffer(&frameBuffer);
		GBuffer::genTexture(&buffDIF, GL_COLOR_ATTACHMENT0, screenSize);
		GBuffer::genTexture(&buffNOR, GL_COLOR_ATTACHMENT1, screenSize);
		GBuffer::genTexture(&buffPOS, GL_COLOR_ATTACHMENT2, screenSize);
		GBuffer::genTexture(&buffSPEC, GL_COLOR_ATTACHMENT3, screenSize);

		GLuint attachments[4] = { GL_COLOR_ATTACHMENT0, 
									GL_COLOR_ATTACHMENT1, 
									GL_COLOR_ATTACHMENT2, 
									GL_COLOR_ATTACHMENT3};

		GBuffer::closeGBufferAndDepth(4, attachments, &depthBuffer, screenSize);
		// reflection Gbuffer
		{
			GBuffer::initFrameBuffer(&reflectionGBuffer);
			GBuffer::genTexture(&buffDIF2, GL_COLOR_ATTACHMENT0, screenSize);
			GBuffer::genTexture(&buffNOR2, GL_COLOR_ATTACHMENT1, screenSize);
			GBuffer::genTexture(&buffPOS2, GL_COLOR_ATTACHMENT2, screenSize);
			GBuffer::genTexture(&buffSPEC2, GL_COLOR_ATTACHMENT3, screenSize);

			GLuint attachments[4] = { GL_COLOR_ATTACHMENT0,
				GL_COLOR_ATTACHMENT1,
				GL_COLOR_ATTACHMENT2,
				GL_COLOR_ATTACHMENT3 };

			GBuffer::closeGBufferAndDepth(4, attachments, &depthBuffer, screenSize);
		}
		{
			GBuffer::initFrameBuffer(&reflectionBuffer);
			GBuffer::genTexture(&buffREFLECTION, GL_COLOR_ATTACHMENT0, screenSize);

			GLuint attachLighting[1] = { GL_COLOR_ATTACHMENT0};

			GBuffer::closeGBufferAndDepth(1, attachLighting, &depthBuffer, screenSize);
		}
		// LightingPass 
		{
			GBuffer::initFrameBuffer(&lihgtingBuffer);
			GBuffer::genTexture(&buffALBEDO, GL_COLOR_ATTACHMENT0, screenSize);
			GBuffer::genTexture(&buffLUMINANCE, GL_COLOR_ATTACHMENT1, screenSize);
			GBuffer::genTexture(&buffREFRACTION, GL_COLOR_ATTACHMENT2, screenSize);

			GLuint attachLighting[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };

			GBuffer::closeGBufferAndDepth(3, attachLighting, &depthBuffer, screenSize);
		}

		// Bloom stack
		{
			// Horitzonal blur
			GBuffer::initFrameBuffer(&bloomBuffer[0]);
			GBuffer::genTexture(&buffBLOOM[0], GL_COLOR_ATTACHMENT0, screenSize);

			GLuint attachmentsH[1] = { GL_COLOR_ATTACHMENT0 };

			GBuffer::closeGBufferAndDepth(1, attachmentsH, &depthBuffer, screenSize);
			
			// Vertical Blur
			GBuffer::initFrameBuffer(&bloomBuffer[1]);
			GBuffer::genTexture(&buffBLOOM[1], GL_COLOR_ATTACHMENT0, screenSize);
			GLuint attachmentsV[1] = { GL_COLOR_ATTACHMENT0 };

			GBuffer::closeGBufferAndDepth(1, attachmentsV, &depthBuffer, screenSize);
		}
		// SKybox
		{
			GBuffer::initFrameBuffer(&skyboxBuffer);
			GBuffer::genTexture(&buffSkybox, GL_COLOR_ATTACHMENT0, screenSize);
			GLuint attachmentsV[1] = { GL_COLOR_ATTACHMENT0 };

			GBuffer::closeGBufferAndDepth(1, attachmentsV, &depthBuffer, screenSize);
		}

		glGenVertexArrays(1, &deferredVAO);
		glGenBuffers(1, &deferredVBO);

		//Generate the VBO if it isn't already generated
		//This is for preventing a memory leak if someone calls twice the init method
		glBindVertexArray(deferredVBO);

		//Bind the buffer object. YOU MUST BIND  the buffer vertex object before binding attributes
		glBindBuffer(GL_ARRAY_BUFFER, deferredVBO);

		//Connect the xyz to the "vertexPosition" attribute of the vertex shader
		glEnableVertexAttribArray(glGetAttribLocation(shaderGBuffer.programID, "vertexPosition"));
		glEnableVertexAttribArray(glGetAttribLocation(shaderGBuffer.programID, "vertexUV"));
		glEnableVertexAttribArray(glGetAttribLocation(shaderGBuffer.programID, "vertexNormal"));


		glVertexAttribPointer(glGetAttribLocation(shaderGBuffer.programID, "vertexPosition"), 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
		glVertexAttribPointer(glGetAttribLocation(shaderGBuffer.programID, "vertexUV"), 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
		glVertexAttribPointer(glGetAttribLocation(shaderGBuffer.programID, "vertexNormal"), 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

		// unbind the VAO and VBO
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	// Generate depth buffer for shadows
	{
		GBuffer::initFrameBuffer(&shadowFBO);
		//GBuffer::genTexture(&shadowTexture, GL_COLOR_ATTACHMENT0, screenSize);

		glGenTextures(1, &shadowTexture);
		glBindTexture(GL_TEXTURE_2D, shadowTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenSize.x, screenSize.y, 0, GL_RGBA, GL_FLOAT, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, shadowTexture, 0);
		
		GLuint attachShadow[1] = { GL_COLOR_ATTACHMENT0};

		GBuffer::closeGBufferAndDepth(1, attachShadow, &depthBuffer, screenSize);
	}
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void compileShaders() {
	std::cout << "compiling shaders" << std::endl;
	water.initShaders();
	// Create GBuffer Shader
	{
		shaderGBuffer = GBuffer::createShader("../resources/shaders/gbuffer.vs", "../resources/shaders/gbuffer.fs");

		// AttRibutes


		GBuffer::linkShaders(shaderGBuffer);
	}
	// Create Deferred Shader

	{
		shaderPBR = GBuffer::createShader("../resources/shaders/quad_transform.vs", "../resources/shaders/pbr.fs");

		GBuffer::addAttribute(shaderPBR, "vertexPosition");
		GBuffer::addAttribute(shaderPBR, "vertexNormal");
		GBuffer::addAttribute(shaderPBR, "verextUV");

		GBuffer::linkShaders(shaderPBR);
	}
	// Create postProcess
	{
		shaderBlur = GBuffer::createShader("../resources/shaders/quad_transform.vs", "../resources/shaders/blur.fshader");

		GBuffer::addAttribute(shaderBlur, "vertPosition");
		GBuffer::addAttribute(shaderBlur, "vertNormal");
		GBuffer::addAttribute(shaderBlur, "vertUV");

		GBuffer::linkShaders(shaderBlur);

	}

	// Final Stack accumulation
	{
		shaderFinal = GBuffer::createShader("../resources/shaders/quad_transform.vs", "../resources/shaders/shader.fs");

		GBuffer::addAttribute(shaderFinal, "vertexPosition");
		GBuffer::addAttribute(shaderFinal, "vertexNormal");
		GBuffer::addAttribute(shaderFinal, "verextUV");

		GBuffer::linkShaders(shaderFinal);

	}
	{
		shaderSkybox = GBuffer::createShader("../resources/shaders/skybox_learn.vs", "../resources/shaders/skybox_learn.fs");
		GBuffer::addAttribute(shaderSkybox, "aPos");
		GBuffer::linkShaders(shaderSkybox);
	}
	{
		shaderShadow = GBuffer::createShader("../resources/shaders/shadow.vs", "../resources/shaders/shadow.fs");
		GBuffer::addAttribute(shaderShadow, "vertPosition");
		GBuffer::linkShaders(shaderShadow);
	}
}

#pragma endregion

#pragma region RENDER


void loadScene(std::string sceneToLoad) {
	if (scene) {
		delete scene;
	}
	scene = new Scene();
	SceneCreator::Instance().createScene(sceneToLoad, *scene);
	water.water = scene->sWater;
}

void sendObject(Vertex *data, GameObject object, int numVertices, Shader shader) {
	auto guard2 = profiler.CreateProfileMarkGuard("Send object");
	glm::mat4 modelMatrix;
	glm::mat3 normalMatrix;

	{
		auto guard2 = profiler.CreateProfileMarkGuard("GLM");

		modelMatrix = glm::translate(modelMatrix, object.translate);

		if (object.angle != 0) {
			modelMatrix = glm::rotate(modelMatrix, glm::radians(object.angle), object.rotation);
		}
		modelMatrix = glm::scale(modelMatrix, object.scale);
		normalMatrix = glm::mat3(glm::transpose(glm::inverse(modelMatrix)));
	}
	
		GBuffer::sendUniform(shader, "modelMatrix", modelMatrix);
		GBuffer::sendUniform(shader, "modelMatrix3x3", glm::mat3(modelMatrix));
		GBuffer::sendUniform(shader, "modelNormalMatrix", normalMatrix);
	{
		auto guard2 = profiler.CreateProfileMarkGuard("data GPU");

		GBuffer::sendDataToGPU(*data, numVertices);
	}
}

void renderScene() {
		GBuffer::bindVertexArrayBindBuffer(gBVAO, gBVBO);
		GBuffer::sendUniform(shaderGBuffer, "textureScaleFactor", glm::vec2(1.0f));
	
	{
		auto guard2 = profiler.CreateProfileMarkGuard("loop scene");

		for (DecorObjects decor : scene->listObjects) {
			
			{
				auto guard1 = profiler.CreateProfileMarkGuard("textures");
				Material m = decor.e->getMaterial();
				GBuffer::sendTexture(shaderGBuffer, "textureData", m.textureMap, GL_TEXTURE0, 0);
				GBuffer::sendTexture(shaderGBuffer, "materialMap", m.metallicMap, GL_TEXTURE1, 1);
				GBuffer::sendTexture(shaderGBuffer, "normalMap", m.normalMap, GL_TEXTURE2, 2);
				GBuffer::sendTexture(shaderGBuffer, "roughness", m.roughnessMap, GL_TEXTURE3, 3);
				GBuffer::sendUniform(shaderGBuffer, "haveMaterialMap", true);
				
			}
			sendObject(decor.e->getMesh(), decor.g.at(0), decor.e->getNumVertices(), shaderGBuffer);

			GBuffer::unbindTextures();
		}
	}

	auto guard3 = profiler.CreateProfileMarkGuard("render terrain");
	GBuffer::sendUniform(shaderGBuffer, "textureScaleFactor", glm::vec2(5.0f));

	GBuffer::sendTexture(shaderGBuffer, "textureData", scene->sTerrain.getMaterial().textureMap, GL_TEXTURE0, 0);
	GBuffer::sendTexture(shaderGBuffer, "materialMap", scene->sTerrain.getMaterial().metallicMap, GL_TEXTURE1, 1);
	GBuffer::sendTexture(shaderGBuffer, "normalMap", scene->sTerrain.getMaterial().normalMap, GL_TEXTURE2, 2);
	GBuffer::sendTexture(shaderGBuffer, "roughness", scene->sTerrain.getMaterial().roughnessMap, GL_TEXTURE3, 3);

	sendObject(scene->sTerrain.getMesh(), scene->sTerrain.getGameObject(), scene->sTerrain.getNumVertices(), shaderGBuffer);
	GBuffer::unbindTextures();

	GBuffer::unbindVertexUnbindBuffer();
}

void renderShadowsScene() {

	GBuffer::bindVertexArrayBindBuffer(gBVAO, gBVBO);
	for (DecorObjects decor : scene->listObjects) {
		sendObject(decor.e->getMesh(), decor.g.at(0), decor.e->getNumVertices(), shaderShadow);
	}

	sendObject(scene->sTerrain.getMesh(), scene->sTerrain.getGameObject(), scene->sTerrain.getNumVertices(), shaderShadow);

	GBuffer::unbindVertexUnbindBuffer();
}

#pragma endregion

enum class postproces {NORMAL, CUBEMAP, PIXELATION, NIGHTVISION, NORMALMAP} postpro;

glm::vec3 dirPosition = {1.0, 0.0, 1.0};
float dColor[] = {1.0, 1.0, 1.0};
float dAngle = { 0.0 };
float dHeight = {0.0};
std::string outputname = "image_name.bmp";

static int debutTexture = 0;
void GUI() {
	water.GUI();
	if (ImGui::CollapsingHeader("GBuffer Water")) {
		ImGui::Image((ImTextureID)buffREFLECTION, ImVec2(screenSize.x / 4, screenSize.y / 4), ImVec2(0.0, 1.0), ImVec2(1.0, 0.0));
		ImGui::Image((ImTextureID)buffNOR2, ImVec2(screenSize.x / 4, screenSize.y / 4), ImVec2(0.0, 1.0), ImVec2(1.0, 0.0));
		ImGui::Image((ImTextureID)buffPOS2, ImVec2(screenSize.x / 4, screenSize.y / 4), ImVec2(0.0, 1.0), ImVec2(1.0, 0.0));
		ImGui::Image((ImTextureID)buffSkybox, ImVec2(screenSize.x / 4, screenSize.y / 4), ImVec2(0.0, 1.0), ImVec2(1.0, 0.0));
	}
	if (ImGui::CollapsingHeader("PBR")) {
		ImGui::Image((ImTextureID)buffALBEDO, ImVec2(screenSize.x / 4, screenSize.y / 4), ImVec2(0.0, 1.0), ImVec2(1.0, 0.0));
	}

	if (ImGui::CollapsingHeader("window")) {
		ImGui::Text("FPS: %f", fps._fps);
		ImGui::Checkbox("limit fps", &fps._enable);
		ImGui::SliderFloat("velocity", &camera.velocity, 0.0f, 0.2f);
	}

	if (ImGui::CollapsingHeader("Light")) {
		if (ImGui::ColorEdit3("dir color", dColor)) {
			directionalColor = glm::vec3(dColor[0], dColor[1], dColor[2]);
		}
		if (ImGui::SliderFloat("dir angle", &dAngle, 0.0f, 360.0f)) {
			scene->sLights.at(0).lPosition.x = 100.0*glm::cos(glm::radians(dAngle));
			scene->sLights.at(0).lPosition.z = 100.0*glm::sin(glm::radians(dAngle));
		}
		if (ImGui::SliderFloat("dir height", &dHeight, 10.0f, 400.0f)) {
			scene->sLights.at(0).lPosition.y = dHeight;
		}
		
		ImGui::ColorEdit3("point color", (float*)&scene->sLights.at(1).lAmbient);
		ImGui::SliderFloat3("point position", (float*)&scene->sLights.at(1).lPosition, -100.0f, 100.0f);
	}

	if (ImGui::CollapsingHeader("Reload")) {
		if (ImGui::Button("Shaders")) compileShaders();
		if (ImGui::Button("Scene"))	loadScene("../resources/scenes/rustic.json");
	}
	//ImGui::SliderFloat("viggneting", &vignneting_radious,  0.0f, 5.0f);
	// posProces
	const char* list_postpro[] = {"normal", "cubemap","pixelation", "night vision", "normal map",  };
	static int e = 0;
	//ImGui::Combo("postproces", &e, list_postpro, IM_ARRAYSIZE(list_postpro));

	switch (e) {
	case 0:
		postpro = postproces::NORMAL;
		break;
	case 1:
		postpro = postproces::CUBEMAP;
		break;
	case 2:
		postpro = postproces::PIXELATION;
		break;
	case 3:
		postpro = postproces::NIGHTVISION;
		break;
	}

	if (ImGui::CollapsingHeader("Debug")) {
		
		const char* list_postpro[] = { "final", "albedo", "world pos","normal", "depth", "material", "skybox"};
		ImGui::Combo("texture", &debutTexture, list_postpro, IM_ARRAYSIZE(list_postpro));
		switch (debutTexture) {
		case 1:
			debugTexture = buffDIF;
			break;
		case 2:
			debugTexture = buffPOS;
			break;
		case 3:
		case 4:
			debugTexture = buffNOR;
			break;
		case 5:
			debugTexture = buffSPEC;
			break;
		case 6:
			debugTexture = buffSkybox;
			break;
		}
		
		if (ImGui::Button("save screenshot")) {
			TextureManager::Instance().saveTexture2File(outputname, screenSize.x, screenSize.y);
		}
	}
}

int main(int argc, char** argv) {

	// Initialize all objects
	//WindowFlags::FULLSCREEN
	window.create("Deferred Shading, Alex Torrents", screenSize.x, screenSize.y, (int)WindowFlags::FULLSCREEN);
	InputManager::Instance().init();
	water.init();
	// IMGUI INIT
	std::string imInit = ImGui_ImplSdlGL3_Init(window.getWindow()) ? "true" : "false";
	std::cout << "Imgui Init " << imInit << std::endl;
	// Init camera
	camera.initializeZBuffer(screenSize);
	camera.setPerspectiveCamera();
	camera.setViewMatrix();
	
	compileShaders();
	initializeVAOVBO();

	glClearColor(0.0, 0.0, 0.0, 1.0);
	loadScene("../resources/scenes/rustic.json");
	glEnable(GL_BLEND);
	bool isOpen = true;
	while (isOpen) {
		fps.startSynchronization();
		// UPDATE
		// Handle inputs
		water.update();

		ImGui_ImplSdlGL3_NewFrame(window.getWindow());
		{
			GUI();
			profiler.DrawProfilerToImGUI(1);
		}
		profiler.AddProfileMark(Utilities::Profiler::MarkerType::BEGIN, 0, "loop");

		{
			if (InputManager::Instance().handleInput() == -1) {
				isOpen = false;
			}
			if (InputManager::Instance().isKeyDown(SDLK_LCTRL)) {
				moveCameraWithKeyboard();
			}
		}
		glEnable(GL_DEPTH_TEST);
		//glEnable(GL_CLIP_DISTANCE0);
		glDisable(GL_BLEND);
		// Geometry pass
		{
			auto guard1 = profiler.CreateProfileMarkGuard("G-buffer");

			// Frame buffer for GBuffer
			glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			
			// GBuffer Pass
			GBuffer::use(shaderGBuffer);

			// send camera to opengl
			GBuffer::sendUniform(shaderGBuffer, "viewMatrix", camera.getViewMatrix());
			GBuffer::sendUniform(shaderGBuffer, "projectionMatrix", camera.getProjectionCamera());
			GBuffer::sendUniform(shaderGBuffer, "viewerPosition", camera.getPosition());
			// write geometry to stencil buffer

			// Send objects
			renderScene();

			// Unbind an unuse programs
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			GBuffer::unuse(shaderGBuffer);
		}
		glEnable(GL_BLEND);
		
		glm::mat4 lighProjection = glm::ortho(-200.0f, 200.0f, -200.0f, 200.0f, 0.1f, 2001.0f);
		glm::mat4 lightView = glm::lookAt(scene->sLights.at(0).lPosition, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		//// SHADOW pass
		{
			auto guard2 = profiler.CreateProfileMarkGuard("shadow");

			glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
			glClear(GL_DEPTH_BUFFER_BIT);

			GBuffer::use(shaderShadow);
			
			GBuffer::sendUniform(shaderShadow, "lightSpaceMatrix", lighProjection * lightView);
			renderShadowsScene();

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			GBuffer::unuse(shaderShadow);
		}
		glDisable(GL_DEPTH_TEST);
		//// SKYBOX
		{		
			auto guard3 = profiler.CreateProfileMarkGuard("skybox");

			glBindFramebuffer(GL_FRAMEBUFFER, skyboxBuffer);
			// skybox	
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// GBuffer Pass
			GBuffer::use(shaderSkybox);

			// send camera to opengl
			GBuffer::sendUniform(shaderSkybox, "view", camera.getViewMatrix());
			GBuffer::sendUniform(shaderSkybox, "projection", camera.getProjectionCamera());
			
			glm::mat4 model = glm::translate(glm::mat4(), camera.getPosition());
			GBuffer::sendUniform(shaderSkybox, "model", model);

			GBuffer::bindVertexArrayBindBuffer(skyboxVAO, skyboxVBO);

			GBuffer::sendCubemap(shaderSkybox, "skybox", scene->sCubemap);

			glDrawArrays(GL_TRIANGLES, 0, 36);
			GBuffer::unbindVertexUnbindBuffer();

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			GBuffer::unuse(shaderSkybox);
		}
		// lighting pass
		{
			auto guard4 = profiler.CreateProfileMarkGuard("PBR - lighting");

			glBindFramebuffer(GL_FRAMEBUFFER, lihgtingBuffer);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Deferred Pass
			GBuffer::use(shaderPBR);

			GBuffer::sendUniform(shaderPBR, "viewerPosition", camera.getPosition());
			GBuffer::sendUniform(shaderPBR, "lightViewMatrix", lightView);
			GBuffer::sendUniform(shaderPBR, "lightProjectionMatrix", lighProjection);
			GBuffer::sendUniform(shaderPBR, "worldViewMatrix", camera.getViewMatrix());
			GBuffer::sendUniform(shaderPBR, "worldProjMatrix", camera.getProjectionCamera());

			GBuffer::sendTexture(shaderPBR, "gDiff", buffDIF, GL_TEXTURE0, 0);
			GBuffer::sendTexture(shaderPBR, "gNorm", buffNOR, GL_TEXTURE1, 1);
			GBuffer::sendTexture(shaderPBR, "gPos", buffPOS, GL_TEXTURE2, 2);
			GBuffer::sendTexture(shaderPBR, "gSpec", buffSPEC, GL_TEXTURE3, 3);
			GBuffer::sendTexture(shaderPBR, "skybox", buffSkybox, GL_TEXTURE4, 4);
			GBuffer::sendTexture(shaderPBR, "shadow", shadowTexture, GL_TEXTURE5, 5);

			GBuffer::sendCubemap(shaderPBR, "cubemap", scene->sCubemap);
			
			GBuffer::sendUniform(shaderPBR, "cubemapActive", postpro == postproces::CUBEMAP ? 1 : 0);
			GBuffer::sendUniform(shaderPBR, "lightSpaceMatrix", lighProjection * lightView);
			
			int count = 0;
			for (Light l : scene->sLights) {
				GBuffer::sendUniform(shaderPBR, "lights["+ std::to_string(count) +"].type", l.getType());
				if (l.getType() == LIGHT_DIRECTIONAL) {
					GBuffer::sendUniform(shaderPBR, "lights[" + std::to_string(count) + "].amb", directionalColor);
					GBuffer::sendUniform(shaderPBR, "lights[" + std::to_string(count) + "].pos", -l.getPosition());
				} else {
					GBuffer::sendUniform(shaderPBR, "lights[" + std::to_string(count) + "].amb", l.getAmbient());
					GBuffer::sendUniform(shaderPBR, "lights[" + std::to_string(count) + "].pos", l.getPosition());
				}
				++count;
			}

			quad.draw();
			GBuffer::unuse(shaderPBR);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		//// Bloom 
		{
			auto guard5 = profiler.CreateProfileMarkGuard("Bloom");

			GBuffer::use(shaderBlur);

			for (int i = 0; i < 2; i++) {
				glBindFramebuffer(GL_FRAMEBUFFER, bloomBuffer[i]);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				if (i == 0) {
					GBuffer::sendTexture(shaderBlur, "gLuminance", buffLUMINANCE, GL_TEXTURE0, 0);
				} else {
					GBuffer::sendTexture(shaderBlur, "gLuminance", buffBLOOM[0], GL_TEXTURE0, 0);
				}
					
				GBuffer::sendUniform(shaderBlur, "blurType", i);

				quad.draw();
			}
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			GBuffer::unuse(shaderBlur);
		}
		
		{
			// REFLECTION
			// G-Buffer
			float distance = 2.0 * (camera.cCameraPos.y - 0.0);
			camera.cCameraPos.y -= distance;
			//camera.up = -1.0;
			camera.pitch = -camera.pitch;
			camera.setViewMatrix();
			glBindFramebuffer(GL_FRAMEBUFFER, reflectionGBuffer);
			{
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				glEnable(GL_CLIP_DISTANCE0);
				glEnable(GL_DEPTH_TEST);
				// GBuffer Pass
				GBuffer::use(shaderGBuffer);

				// send camera to opengl
				GBuffer::sendUniform(shaderGBuffer, "viewMatrix", camera.getViewMatrix());
				GBuffer::sendUniform(shaderGBuffer, "projectionMatrix", camera.getProjectionCamera());
				GBuffer::sendUniform(shaderGBuffer, "viewerPosition", camera.getPosition());
				// write geometry to stencil buffer

				// Send objects
				renderScene();

				// Unbind an unuse programs
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				GBuffer::unuse(shaderGBuffer);

			}
			glDisable(GL_DEPTH_TEST);
			glDisable(GL_CLIP_DISTANCE0);
			{

				glBindFramebuffer(GL_FRAMEBUFFER, skyboxBuffer);
				// skybox	
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				// GBuffer Pass
				GBuffer::use(shaderSkybox);

				// send camera to opengl
				GBuffer::sendUniform(shaderSkybox, "view", camera.getViewMatrix());
				GBuffer::sendUniform(shaderSkybox, "projection", camera.getProjectionCamera());

				glm::mat4 model = glm::translate(glm::mat4(), camera.getPosition());
				GBuffer::sendUniform(shaderSkybox, "model", model);

				GBuffer::bindVertexArrayBindBuffer(skyboxVAO, skyboxVBO);

				GBuffer::sendCubemap(shaderSkybox, "skybox", scene->sCubemap);

				glDrawArrays(GL_TRIANGLES, 0, 36);
				GBuffer::unbindVertexUnbindBuffer();

				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				GBuffer::unuse(shaderSkybox);
			}
			{
				auto guard4 = profiler.CreateProfileMarkGuard("PBR - lighting");

				glBindFramebuffer(GL_FRAMEBUFFER, reflectionBuffer);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				// Deferred Pass
				GBuffer::use(shaderPBR);

				GBuffer::sendUniform(shaderPBR, "viewerPosition", camera.getPosition());
				GBuffer::sendUniform(shaderPBR, "lightViewMatrix", lightView);
				GBuffer::sendUniform(shaderPBR, "lightProjectionMatrix", lighProjection);
				GBuffer::sendUniform(shaderPBR, "worldViewMatrix", camera.getViewMatrix());
				GBuffer::sendUniform(shaderPBR, "worldProjMatrix", camera.getProjectionCamera());

				GBuffer::sendTexture(shaderPBR, "gDiff", buffDIF2, GL_TEXTURE0, 0);
				GBuffer::sendTexture(shaderPBR, "gNorm", buffNOR2, GL_TEXTURE1, 1);
				GBuffer::sendTexture(shaderPBR, "gPos", buffPOS2, GL_TEXTURE2, 2);
				GBuffer::sendTexture(shaderPBR, "gSpec", buffSPEC2, GL_TEXTURE3, 3);

				GBuffer::sendCubemap(shaderPBR, "cubemap", scene->sCubemap);

				int count = 0;
				for (Light l : scene->sLights) {
					GBuffer::sendUniform(shaderPBR, "lights[" + std::to_string(count) + "].type", l.getType());
					if (l.getType() == LIGHT_DIRECTIONAL) {
						GBuffer::sendUniform(shaderPBR, "lights[" + std::to_string(count) + "].amb", directionalColor);
						GBuffer::sendUniform(shaderPBR, "lights[" + std::to_string(count) + "].pos", -l.getPosition());
					}
					else {
						GBuffer::sendUniform(shaderPBR, "lights[" + std::to_string(count) + "].amb", l.getAmbient());
						GBuffer::sendUniform(shaderPBR, "lights[" + std::to_string(count) + "].pos", l.getPosition());
					}
					++count;
				}

				quad.draw();
				GBuffer::unuse(shaderPBR);
			}
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			camera.cCameraPos.y += distance;
			camera.pitch = -camera.pitch;
			//camera.up = +1.0;

			camera.setViewMatrix();
		}
		// Final stack
		{
			auto guard6 = profiler.CreateProfileMarkGuard("Final stack");

			GBuffer::use(shaderFinal);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			
			GBuffer::sendTexture(shaderFinal, "gColor", buffALBEDO, GL_TEXTURE0, 0);
			GBuffer::sendTexture(shaderFinal, "gBloom", buffBLOOM[1], GL_TEXTURE1, 1);
			GBuffer::sendTexture(shaderFinal, "debugTexture", debugTexture, GL_TEXTURE2, 2);
			//GBuffer::sendTexture(shaderFinal, "water", water.texture, GL_TEXTURE3, 3);
			GBuffer::sendUniform(shaderFinal, "debugMode", debutTexture);

			GBuffer::sendUniform(shaderFinal, "pixelation", postpro == postproces::PIXELATION ? 1 : 0);
			GBuffer::sendUniform(shaderFinal, "nightVision", postpro == postproces::NIGHTVISION ? 1 : 0);

			quad.draw();

			GBuffer::unuse(shaderFinal);
		}
		{
			auto guard5 = profiler.CreateProfileMarkGuard("Water");
			glBindFramebuffer(GL_READ_FRAMEBUFFER, frameBuffer);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
			glBlitFramebuffer(
				0, 0, screenSize.x, screenSize.y, 0, 0, screenSize.x, screenSize.y, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			water.draw();
		}
		{
			auto guard6 = profiler.CreateProfileMarkGuard("Imgui Render");

			ImGui::Render();
		}
		{
			auto guard6 = profiler.CreateProfileMarkGuard("sync");

			fps.forceSynchronization();
		}
		{
			auto guard6 = profiler.CreateProfileMarkGuard("swap buffer");
			window.swapBuffer();
		}
		
		profiler.AddProfileMark(Utilities::Profiler::MarkerType::END, 0, "loop");
	}

	ImGui_ImplSdlGL3_Shutdown();

	delete scene;

	return 0;
}

void moveCameraWithKeyboard() {
	// Rotate the camera with mouse
	if (InputManager::Instance().mousePressed()) {
		SDL_SetRelativeMouseMode(SDL_TRUE);
		
		glm::vec2 mousePos = InputManager::Instance().getMouseCoords() - glm::ivec2(screenSize.x/2, screenSize.y / 2);
		camera.rotate(mousePos);
	} else {
		SDL_SetRelativeMouseMode(SDL_FALSE);
		camera.resetFirstEntry();
	}

	if (InputManager::Instance().isKeyDown(SDLK_e)) {
		camera.moveTo(CameraMove::up);
	}
	if (InputManager::Instance().isKeyDown(SDLK_q)) {
		camera.moveTo(CameraMove::down);
	}
	if (InputManager::Instance().isKeyDown(SDLK_w)) {
		camera.moveTo(CameraMove::forward);
	}
	if (InputManager::Instance().isKeyDown(SDLK_s)) {
		camera.moveTo(CameraMove::backwards);
	}
	if (InputManager::Instance().isKeyDown(SDLK_a)) {
		camera.moveTo(CameraMove::left);
	}
	if (InputManager::Instance().isKeyDown(SDLK_d)) {
		camera.moveTo(CameraMove::right);
	}
}