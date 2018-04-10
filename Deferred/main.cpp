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



#define IM_ARRAYSIZE(_ARR)  ((int)(sizeof(_ARR)/sizeof(*_ARR)))

//const glm::vec2 screenSize = {960, 540};
const glm::vec2 screenSize = { 1920, 1080};
//
Camera camera;
FPSLimiter fps = FPSLimiter(true, 60, false);
Window window;

Shader shaderGBuffer;
Shader shaderPBR;
Shader shaderBlur;
Shader shaderFinal;
Shader shaderSkybox;

Scene *scene;

GLuint deferredVAO , deferredVBO;
GLuint gBVAO, gBVBO;
GLuint skyboxVAO, skyboxVBO;
GLuint frameBuffer;
GLuint buffNOR, buffDIF, buffPOS, buffSPEC;
GLuint depthBuffer;

// Lighting Pass
GLuint lihgtingBuffer, buffALBEDO, buffLUMINANCE;

// Skybox
GLuint skyboxBuffer, buffSkybox;
// Final stack pass
GLuint bloomBuffer[2];
GLuint buffBLOOM[2];


void moveCameraWithKeyboard();

struct Quad {
	OBJ object;
	Quad() {
		object = Geometry::LoadModelFromFile("../resources/objects/quad.obj");
	}

	void draw() {
		GBuffer::bindVertexArrayBindBuffer(deferredVAO, deferredVBO);
		GBuffer::sendDataToGPU(object.mesh, object.numVertices);
		GBuffer::unbindVertexUnbindBuffer();
	}
	
} quad;

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

		// LightingPass 
		{
			GBuffer::initFrameBuffer(&lihgtingBuffer);
			GBuffer::genTexture(&buffALBEDO, GL_COLOR_ATTACHMENT0, screenSize);
			GBuffer::genTexture(&buffLUMINANCE, GL_COLOR_ATTACHMENT1, screenSize);

			GLuint attachLighting[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};

			GBuffer::closeGBufferAndDepth(2, attachLighting, &depthBuffer, screenSize);
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


	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void compileShaders() {
	std::cout << "compiling shaders" << std::endl;
	// Create GBuffer Shader
	{
		shaderGBuffer = GBuffer::createShader("../resources/shaders/gbuffer.vshader", "../resources/shaders/gbuffer.fshader");

		// AttRibutes
		GBuffer::addAttribute(shaderGBuffer, "vertPosition");
		GBuffer::addAttribute(shaderGBuffer, "vertNormal");
		GBuffer::addAttribute(shaderGBuffer, "vertUV");

		GBuffer::linkShaders(shaderGBuffer);
	}
	// Create Deferred Shader

	{
		shaderPBR = GBuffer::createShader("../resources/shaders/quad_transform.vshader", "../resources/shaders/pbr.fshader");

		GBuffer::addAttribute(shaderPBR, "vertPosition");
		GBuffer::addAttribute(shaderPBR, "vertNormal");
		GBuffer::addAttribute(shaderPBR, "vertUV");

		GBuffer::linkShaders(shaderPBR);
	}
	// Create postProcess
	{
		shaderBlur = GBuffer::createShader("../resources/shaders/quad_transform.vshader", "../resources/shaders/blur.fshader");

		GBuffer::addAttribute(shaderBlur, "vertPosition");
		GBuffer::addAttribute(shaderBlur, "vertNormal");
		GBuffer::addAttribute(shaderBlur, "vertUV");

		GBuffer::linkShaders(shaderBlur);

	}

	// Final Stack accumulation
	{
		shaderFinal = GBuffer::createShader("../resources/shaders/quad_transform.vshader", "../resources/shaders/shader.fshader");

		GBuffer::addAttribute(shaderFinal, "vertPosition");
		GBuffer::addAttribute(shaderFinal, "vertNormal");
		GBuffer::addAttribute(shaderFinal, "vertUV");

		GBuffer::linkShaders(shaderFinal);

	}
	{
		shaderSkybox = GBuffer::createShader("../resources/shaders/skybox_learn.vs", "../resources/shaders/skybox_learn.fs");
		GBuffer::addAttribute(shaderSkybox, "aPos");
		GBuffer::linkShaders(shaderSkybox);
	}

}

void loadScene(std::string sceneToLoad) {
	if (scene) {
		delete scene;
	}
	scene = new Scene();
	SceneCreator::Instance().createScene(sceneToLoad, *scene);
}

void sendObject(Vertex *data, GameObject object, int numVertices) {
	glm::mat4 modelMatrix;
	glm::mat3 normalMatrix;

	modelMatrix = glm::translate(modelMatrix, object.translate);

	if (object.angle != 0) {
		modelMatrix = glm::rotate(modelMatrix, glm::radians(object.angle), object.rotation);
	}
	modelMatrix = glm::scale(modelMatrix, object.scale);
	normalMatrix = glm::mat3(glm::transpose(glm::inverse(modelMatrix)));

	GBuffer::sendUniform(shaderGBuffer, "modelMatrix", modelMatrix);
	GBuffer::sendUniform(shaderGBuffer, "modelMatrix3x3", glm::mat3(modelMatrix));
	GBuffer::sendUniform(shaderGBuffer, "modelNormalMatrix", normalMatrix);

	GBuffer::sendDataToGPU(data, numVertices);
}

void renderScene() {

	GBuffer::bindVertexArrayBindBuffer(gBVAO, gBVBO);

	GBuffer::sendUniform(shaderGBuffer, "textureScaleFactor", glm::vec2(1.0f));

	for (DecorObjects decor : scene->listObjects) {
		GBuffer::sendTexture(shaderGBuffer, "textureData", decor.e->getMaterial().textureMap, GL_TEXTURE0, 0);
		if (decor.e->getMaterial().specularMap != -1) {
			GBuffer::sendTexture(shaderGBuffer, "materialMap", decor.e->getMaterial().specularMap, GL_TEXTURE1, 1);
			GBuffer::sendTexture(shaderGBuffer, "normalMap", decor.e->getMaterial().normalMap, GL_TEXTURE2, 2);
			GBuffer::sendUniform(shaderGBuffer, "haveMaterialMap", true);
		} else {
			GBuffer::sendUniform(shaderGBuffer, "haveMaterialMap", false);
		}

		sendObject(decor.e->getMesh(), decor.g.at(0), decor.e->getNumVertices());

		//GBuffer::unbindTextures();
	}

	/*GBuffer::sendUniform(shaderGBuffer, "textureScaleFactor", glm::vec2(10.0f));
	GBuffer::sendTexture(shaderGBuffer, "textureData", scene->sTerrain.getMaterial().textureMap, GL_TEXTURE0, 0);
	GBuffer::sendTexture(shaderGBuffer, "materialMap", scene->sTerrain.getMaterial().specularMap, GL_TEXTURE1, 1);
	GBuffer::sendTexture(shaderGBuffer, "normalMap", scene->sTerrain.getMaterial().normalMap, GL_TEXTURE2, 2);
	GBuffer::sendUniform(shaderGBuffer, "haveMaterialMap", true);
	sendObject(scene->sTerrain.getMesh(), scene->sTerrain.getGameObject(), scene->sTerrain.getNumVertices());*/
	GBuffer::unbindTextures();

	GBuffer::unbindVertexUnbindBuffer();
}


enum class postproces {NORMAL, CUBEMAP, PIXELATION, NIGHTVISION, NORMALMAP} postpro;

glm::vec3 directionalColor = { 1.0, 1.0, 1.0 };
float dColor[] = {0.0, 1.0, 0.0};
float dPos[] = {0.0, 0.0, 0.0};

void GUI() {
	if (ImGui::CollapsingHeader("window")) {
		ImGui::Text("FPS: %f", fps._fps);
		ImGui::Checkbox("limit fps", &fps._enable);
		ImGui::SliderFloat("velocity", &camera.velocity, 0.0f, 0.2f);

	}

	if (ImGui::CollapsingHeader("directional light")) {
		if (ImGui::ColorEdit3("color", dColor)) {
			directionalColor = glm::vec3(dColor[0], dColor[1], dColor[2]);
		}
		ImGui::SliderFloat3("direction", (float*)&scene->sLights[0].getPosition(), -1.0f, 1.0f);

	}
	if (ImGui::CollapsingHeader("Reload")) {
		if (ImGui::Button("Shaders")) compileShaders();
		if (ImGui::Button("Scene"))	loadScene("../resources/scenes/scene2.json");
	}
	//ImGui::SliderFloat("viggneting", &vignneting_radious,  0.0f, 5.0f);
	// posProces
	const char* list_postpro[] = {"normal", "cubemap","pixelation", "night vision", "normal map" };
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
}

int main(int argc, char** argv) {
	postpro = postproces::NORMAL;

	// Initialize all objects
	
	window.create("Deferred Shading, Alex Torrents", screenSize.x, screenSize.y, 0);
	InputManager::Instance().init();

	// Init camera
	camera.initializeZBuffer(screenSize);
	camera.setPerspectiveCamera();
	camera.setViewMatrix();
	
	compileShaders();
	initializeVAOVBO();

	glClearColor(0.0, 0.0, 0.0, 1.0);
	loadScene("../resources/scenes/scene2.json");
	
	// IMGUI INIT
	std::string imInit = ImGui_ImplSdlGL3_Init(window.getWindow()) ? "true" : "false";
	std::cout << "Imgui Init " << imInit << std::endl;

	glEnable(GL_DEPTH_TEST);

	GLuint roughness = TextureManager::Instance().getTextureID("../resources/images/rusted_iron/roughness.png");

	bool isOpen = true;
	while (isOpen) {
		fps.startSynchronization();
		// UPDATE
		// Handle inputs

		ImGui_ImplSdlGL3_NewFrame(window.getWindow());
		GUI();

		{
			if (InputManager::Instance().handleInput() == -1) {
				isOpen = false;
			}
			if (InputManager::Instance().isKeyDown(SDLK_LCTRL)) {
				moveCameraWithKeyboard();
			}
		}

		// Geometry pass
		{					
			glClear(GL_STENCIL_BUFFER_BIT);
			// Frame buffer for GBuffer
			glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );


			// GBuffer Pass
			GBuffer::use(shaderGBuffer);

			// send camera to opengl
			GBuffer::sendUniform(shaderGBuffer, "viewMatrix", camera.getViewMatrix());
			GBuffer::sendUniform(shaderGBuffer, "projectionMatrix", camera.getProjectionCamera());
			GBuffer::sendUniform(shaderGBuffer, "viewerPosition", camera.getPosition());
			GBuffer::sendTexture(shaderGBuffer, "roughness", roughness, GL_TEXTURE3, 3);
			// write geometry to stencil buffer
			
			// Send objects
			renderScene();

			// Unbind an unuse programs
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			GBuffer::unuse(shaderGBuffer);
		}
		
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
		
		// lighting pass
		{
			glBindFramebuffer(GL_FRAMEBUFFER, lihgtingBuffer);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Deferred Pass
			GBuffer::use(shaderPBR);

			GBuffer::sendUniform(shaderPBR, "viewerPosition", camera.getPosition());
			GBuffer::sendTexture(shaderPBR, "gDiff", buffDIF, GL_TEXTURE0, 0);
			GBuffer::sendTexture(shaderPBR, "gNorm", buffNOR, GL_TEXTURE1, 1);
			GBuffer::sendTexture(shaderPBR, "gPos", buffPOS, GL_TEXTURE2, 2);
			GBuffer::sendTexture(shaderPBR, "gSpec", buffSPEC, GL_TEXTURE3, 3);
			GBuffer::sendTexture(shaderPBR, "skybox", buffSkybox, GL_TEXTURE4, 4);

			GBuffer::sendCubemap(shaderPBR, "cubemap", scene->sCubemap);
			
			GBuffer::sendUniform(shaderPBR, "cubemapActive", postpro == postproces::CUBEMAP ? 1 : 0);
			
			int count = 0;
			for (Light l : scene->sLights) {
				GBuffer::sendUniform(shaderPBR, "lights["+ std::to_string(count) +"].type", l.getType());
				if (l.getType() == LIGHT_DIRECTIONAL) {
					GBuffer::sendUniform(shaderPBR, "lights[" + std::to_string(count) + "].amb", directionalColor);
				} else {
					GBuffer::sendUniform(shaderPBR, "lights[" + std::to_string(count) + "].amb", l.getAmbient());
				}
				GBuffer::sendUniform(shaderPBR, "lights[" + std::to_string(count) + "].pos", l.getPosition());
				++count;
			}

			quad.draw();
			GBuffer::unuse(shaderPBR);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
		// Bloom 
		{
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
		// Final stack
		{
			GBuffer::use(shaderFinal);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			GBuffer::sendTexture(shaderFinal, "gColor", buffALBEDO, GL_TEXTURE0, 0);
			GBuffer::sendTexture(shaderFinal, "gBloom", buffBLOOM[1], GL_TEXTURE1, 1);

			GBuffer::sendUniform(shaderFinal, "pixelation", postpro == postproces::PIXELATION ? 1 : 0);
			GBuffer::sendUniform(shaderFinal, "nightVision", postpro == postproces::NIGHTVISION ? 1 : 0);
			//GBuffer::sendUniform(shaderFinal, "vignneting_radious", vignneting_radious);

			quad.draw();

			GBuffer::unuse(shaderFinal);
		}
		
		
		ImGui::Render();

		window.swapBuffer();

		fps.forceSynchronization();
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