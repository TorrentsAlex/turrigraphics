#ifndef TURRI_OBJ
#define TURRI_OBJ

#include <glm/glm.hpp>
#include <vector>
#include "Vertex.h"
#include "TextureManager.h"
using namespace std;

struct OBJ {
	// Original model
	vector<float> faces;
	vector<float> uv;
	vector<float> normals;
	vector<float> tangent;
	vector<float> bitangent;

	// readed from file
	vector<glm::vec3> vertexs;
	vector<glm::vec2> textures_coord;
	vector<glm::vec3> vertex_normals;

	glm::vec2 lenght;
	glm::vec2 width;
	glm::vec2 high;

	Vertex* mesh;
	int numVertices;
};
#endif
