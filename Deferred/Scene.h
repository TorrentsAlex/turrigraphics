#ifndef TURRI_SCENE
#define TURRI_SCENE

#include "Entity.h"

#include "Light.h"
#include <vector>

using namespace std;
struct DecorObjects {
	Entity* e;
	vector<GameObject> g;
};

class Scene {
private:
	Entity sSkybox;
	Entity sTerrain;

	/* Variable deprecated, using DecorObjects for render */
	vector<Entity> sDecoration;

	GLuint sCubemap;

public:
	Scene();
	~Scene();
	vector<Light> sLights;

	vector<DecorObjects> listObjects;
	// Setters
	void setSkyBox(OBJ object, GLuint texture);
	void setTerrain(OBJ object, GLuint texture, Material material);
	void setDecoration(vector<Entity> decoration);
	void setLights(vector<Light> lights);
	void setCubemap(GLuint cubemap);

	// Getters
	Entity getTerrain();
	Entity getSkyBox();
	vector<Entity> getDecoration();
	vector<Light> &getLights();
	GLuint getCubemap();

	void clean();
	void update();
};

#endif
