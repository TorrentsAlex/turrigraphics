#ifndef SCENE_CREATOR
#define SCENE_CREATOR

#include <string>
#include <vector>
#include <iostream>

#include "FileReader.h"
#include "Geometry.h"
#include "json/json.h"

#include "Scene.h"

#include "Light.h"

using namespace std;

class SceneCreator {
private:
	Material metalMaterial;
	SceneCreator() {
		metalMaterial.ambient = glm::vec3(0.25, 0.20725, 0.20725);
		metalMaterial.diffuse = glm::vec3(0.25, 0.20725, 0.20725);
		metalMaterial.specular = glm::vec3(0.25, 0.20725, 0.20725);
		metalMaterial.shininess = 4;
	};

	vector<Light> populateLights(Json::Value lights);
	void populateDecoration(Scene* scene, Json::Value decoration);
	void populateTerrain(Scene* scene, Json::Value terrain);

public:
	static SceneCreator& Instance() {
		static SceneCreator instance;	// Guaranteed to be destroyed.
										// Instantiated on first use.
		return instance;
	}
	// Singleton Pattern - Delete Copy constructor and operator=
	SceneCreator(SceneCreator const&) = delete;
	void operator=(SceneCreator const&) = delete;

	void createScene(string file, Scene& newScene);
};

#endif
