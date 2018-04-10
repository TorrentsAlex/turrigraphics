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
public:
	Entity sSkybox;
	Entity sTerrain;

	GLuint sCubemap;

	Scene();
	~Scene();
	vector<Light> sLights;

	vector<DecorObjects> listObjects;
	// Setters
	void setSkyBox(OBJ object, GLuint texture);
	void setTerrain(OBJ object, GLuint texture, Material material);
	void setLights(vector<Light> lights);
	void setCubemap(GLuint cubemap);

	// Getters
	/*Entity getTerrain();
	Entity getSkyBox();
	vector<Light> &getLights();
	GLuint getCubemap();*/

	void clean();
	void update();
};

#endif
