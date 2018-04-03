#include "SceneCreator.h"


void SceneCreator::createScene(string file, Scene& newScene) {
	cout << "creating scene" << endl;
	Json::Reader reader;
	Json::Value json;

	// Load File and save it into string
	string jsonString = FileReader::LoadStringFromFile(file);
	// Parse the string into json
	reader.parse(jsonString, json);

	// Terrain and skybox
	populateTerrain(&newScene, json);

	populateDecoration(&newScene, json);
	
	// Lights
	vector<Light> sceneLights = populateLights(json["lights"]);
	newScene.setLights(sceneLights);
}

/**
	Create a vector of lights with the json Scene
*/
vector<Light> SceneCreator::populateLights(Json::Value jsonLights) {
	Json::Reader reader;
	vector<Light> sceneLights;

	for (auto i = 0; i < jsonLights.size(); i++) {
		Light l;
		glm::vec3 ambient, diffuse, specular, position, direction;

		Json::Value currentLight;
		string stringlight = FileReader::LoadStringFromFile(jsonLights[i].asString());
		// Parse the string into json
		reader.parse(stringlight, currentLight);

		// ambient
		ambient.r = currentLight["ambient"]["r"].asFloat();
		ambient.g = currentLight["ambient"]["g"].asFloat();
		ambient.b = currentLight["ambient"]["b"].asFloat();

		diffuse.r = currentLight["diffuse"]["r"].asFloat();
		diffuse.g = currentLight["diffuse"]["g"].asFloat();
		diffuse.b = currentLight["diffuse"]["b"].asFloat();

		specular.r = currentLight["specular"]["r"].asFloat();
		specular.g = currentLight["specular"]["g"].asFloat();
		specular.b = currentLight["specular"]["b"].asFloat();

		// position
		position.x = currentLight["position"]["x"].asFloat();
		position.y = currentLight["position"]["y"].asFloat();
		position.z = currentLight["position"]["z"].asFloat();

		// direction
		direction.x = currentLight["direction"]["x"].asFloat();
		direction.y = currentLight["direction"]["y"].asFloat();
		direction.z = currentLight["direction"]["z"].asFloat();

		// type
		string type = currentLight["type"].asString();
		l.setType(type);
		// if the light is a pointlight
		if (type.compare("point") == 0) {
			float constant = currentLight["constant"].asFloat();
			float linear = currentLight["linear"].asFloat();
			float quadratic = currentLight["quadratic"].asFloat();
			l.setConstant(constant);
			l.setLinear(linear);
			l.setQuadratic(quadratic);
		}
		//power
		float power = currentLight["power"].asFloat();
		l.setPower(power);

		// set the values into the light
		l.setAmbient(ambient);
		l.setDiffuse(diffuse);
		l.setSpecular(specular);
		l.setDirection(direction);
		l.setPosition(position);

		sceneLights.push_back(l);
	}
	return sceneLights;
}


void SceneCreator::populateDecoration(Scene * scene, Json::Value decoration) {

	cout << "decoration..." << endl;
	Json::Value jsonDecoration = decoration["decoration"];
	// Decoration
	vector<Entity> vEntityDecorations;
	int size = jsonDecoration.size();
	for (auto i = 0; i < size; i++) {
		Json::Value currentDecoration = jsonDecoration[i];
		OBJ objDecoration = Geometry::LoadModelFromFile(currentDecoration["object"].asString());
		GLuint textureDecoration = TextureManager::Instance().getTextureID(currentDecoration["texture"].asString());

		string gameElements = currentDecoration["elements"].asString();
		string textureSpecularString = currentDecoration["specularMap"].asString();
		string normalString = currentDecoration["normalMap"].asString();

		Entity* e = new Entity();
		e->setOBJ(objDecoration);
		e->setMaterial(metalMaterial);
		e->setTextureId(textureDecoration);

		e->setId(currentDecoration["name"].asString());
		DecorObjects d;
		d.e = e;

		//set specular material
		if (textureSpecularString.compare("") != 0) {
			GLuint specular = TextureManager::Instance().getTextureID(textureSpecularString);
			GLuint normalMap = TextureManager::Instance().getTextureID(normalString);
			e->setTextureSpecular(specular);
			e->eMaterial.normalMap = normalMap;
		}

		GameObject gameObject;
		gameObject.translate = glm::vec3(currentDecoration["position"]["x"].asFloat(), 
										currentDecoration["position"]["y"].asFloat(), 
										currentDecoration["position"]["z"].asFloat());;

		gameObject.scale = glm::vec3(1.0f);
		gameObject.angle = 0;

		std::vector<GameObject> gameObjects;
		gameObjects.push_back(gameObject);
		d.g.push_back(gameObject);

		scene->listObjects.push_back(d);
	}
}


void SceneCreator::populateTerrain(Scene * scene, Json::Value terrain) {

	cout << "skybox..." << endl;
	// Terrain
	OBJ objSkybox = Geometry::LoadModelFromFile(terrain["skybox"]["object"].asString());
	GLuint textureSkybox = TextureManager::Instance().getTextureID(terrain["skybox"]["texture"].asString());

	scene->setSkyBox(objSkybox, textureSkybox);

	std::vector<std::string> cubemapsPaths;
	Json::Value cubemaps = terrain["skybox"]["cubemap"];
	if (cubemaps.size() == 6) {
		for (size_t i = 0; i < 6; i++) {				
				cubemapsPaths.push_back(cubemaps[i].asString());
		}
		GLuint cmId = TextureManager::Instance().getTextureCubemapID(cubemapsPaths);
		scene->setCubemap(cmId);
	}
	cout << "terrain..." << endl;

	// Terrain
	OBJ objTerrain = Geometry::LoadModelFromFile(terrain["terrain"]["object"].asString());

	GLuint textureTerrain = TextureManager::Instance().getTextureID(terrain["terrain"]["texture"].asString());
	GLuint normalTerrain = TextureManager::Instance().getTextureID(terrain["terrain"]["texture"].asString());

	GLuint textureSpecular = TextureManager::Instance().getTextureID(terrain["terrain"]["specularMap"].asString());

	metalMaterial.specularMap = textureSpecular;
	metalMaterial.normalMap = normalTerrain;
	scene->setTerrain(objTerrain, textureTerrain, metalMaterial);

	metalMaterial.specularMap = -1;

}
