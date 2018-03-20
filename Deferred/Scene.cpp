#include "Scene.h"

Scene::Scene() {}

Scene::~Scene() {}

/* Deprecated in Deferred Shading*/
void Scene::setSkyBox(OBJ object, GLuint texture) {
	GameObject skyObject;
	skyObject.angle = 0;
	skyObject.translate = glm::vec3(0, 0, 0);
	skyObject.scale = glm::vec3(7, 7, 7);
	skyObject.rotation = glm::vec3(0, 0, 0);

	sSkybox.setOBJ(object);
	sSkybox.setTextureId(texture);
	sSkybox.setGameObject(skyObject);
}

void Scene::setTerrain(OBJ object, GLuint texture, Material material) {
	GameObject terrainObject;
	terrainObject.angle = 0;
	terrainObject.translate = glm::vec3(0.0f, 0.0f, 0.0f);
	terrainObject.scale = glm::vec3(1.0f, 1.0f, 1.0f);
	terrainObject.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
	sTerrain.setId("terrain");
	sTerrain.setOBJ(object);
	sTerrain.setMaterial(material);
	sTerrain.setTextureId(texture);
	sTerrain.setGameObject(terrainObject);
}

void Scene::setDecoration(std::vector<Entity> decoration) {
	sDecoration = decoration;
}

void Scene::setLights(vector<Light> lights) {
	sLights = lights;
}

void Scene::setCubemap(GLuint cubemap) {
	sCubemap = cubemap;
}

// Getters
Entity Scene::getTerrain() {
	return sTerrain;
}

Entity Scene::getSkyBox() {
	return sSkybox;
}

std::vector<Entity> Scene::getDecoration() {
	return sDecoration;
}

vector<Light>& Scene::getLights() {
	return sLights;
}

GLuint Scene::getCubemap() {
	return sCubemap;
}


void Scene::clean() {

}

//
void Scene::update() {
}
