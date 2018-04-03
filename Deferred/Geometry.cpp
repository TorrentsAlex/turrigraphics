#include "Geometry.h"
#include <iostream>
#include <fstream>

using namespace std;

OBJ Geometry::LoadModelFromFile(std::string file) {
	float xMin = 5000, xMax = 0, yMin = 50000, yMax = 0, zMin = 50000, zMax = 0;
	glm::vec3 currentVertex;
	// read the file and add the vertex and the faces into the object
	OBJ object = FileReader::LoadOBJFromFile(file);

	// Create the object with the vertices and faces
	object.mesh = new Vertex[object.faces.size()];
	object.numVertices = object.faces.size();
	float v1, vt1, vn1;
	int iDebug;

	try {
		for (int i = 0; i < object.faces.size(); i++) {
			iDebug = i;
			v1 = object.faces.at(i);
			vt1 = object.uv.at(i);
			vn1 = object.normals.at(i);

			object.mesh[i].setPosition(object.vertexs.at(v1).x, object.vertexs.at(v1).y, object.vertexs.at(v1).z);
			object.mesh[i].setUV(object.textures_coord.at(vt1).x, object.textures_coord.at(vt1).y);
			object.mesh[i].setNormal(object.vertex_normals.at(vn1).x, object.vertex_normals.at(vn1).y, object.vertex_normals.at(vn1).z);
		}
		// Calculate tangents and bitangents
		for (int i = 0; i < object.faces.size(); i += 3) {
			glm::vec3 v0, v1, v2;
			v0 = object.mesh[i + 0].position.toVec3();
			v1 = object.mesh[i + 1].position.toVec3();
			v2 = object.mesh[i + 2].position.toVec3();

			glm::vec2 uv0, uv1, uv2;
			uv0 = object.mesh[i + 0].uv.toVec2();
			uv1 = object.mesh[i + 1].uv.toVec2();
			uv2 = object.mesh[i + 2].uv.toVec2();

			glm::vec3 deltaPos1 = v1 - v0;
			glm::vec3 deltaPos2 = v2 - v0;
			glm::vec2 deltaUV1 = uv1 - uv0;
			glm::vec2 deltaUV2 = uv2 - uv0;

			// calculate tangent and bitangent
			float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
			glm::vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;
			glm::vec3 bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x) * r;

			object.mesh[i + 0].setTangent(tangent.x, tangent.y, tangent.z);
			object.mesh[i + 1].setTangent(tangent.x, tangent.y, tangent.z);
			object.mesh[i + 2].setTangent(tangent.x, tangent.y, tangent.z);
			
			object.mesh[i + 0].setBitangent(bitangent.x, bitangent.y, bitangent.z);
			object.mesh[i + 1].setBitangent(bitangent.x, bitangent.y, bitangent.z);
			object.mesh[i + 2].setBitangent(bitangent.x, bitangent.y, bitangent.z);

		}
	} catch (std::exception e) {
		std::cerr << "ERROR: " << iDebug << " i " << e.what() << std::endl;
	}

	// calculate the size of the object, not used now..
	// We only need to know the vertex, the faces don't
	for (int i = 0; i < object.vertexs.size(); i++) {
		currentVertex = object.vertexs.at(i);
		// X
		if (currentVertex.x > xMax) {
			xMax = currentVertex.x;
		}
		if (currentVertex.x < xMin) {
			xMin = currentVertex.x;
		}
		// Y
		if (currentVertex.y > yMax) {
			yMax = currentVertex.y;
		}
		if (currentVertex.y < yMin) {
			yMin = currentVertex.y;
		}
		// Z
		if (currentVertex.z > zMax) {
			zMax = currentVertex.z;
		}
		if (currentVertex.z < zMin) {
			zMin = currentVertex.z;
		}

	}
	object.width.x = xMin;
	object.width.y = xMax;
	object.lenght.x = yMin;
	object.lenght.y = yMax;
	object.high.x = zMin;
	object.high.y = zMax;

	return object;
}

std::vector<GameObject> Geometry::LoadGameElements(std::string file) {
	std::vector<GameObject> vGObject;
	GameObject tempObject;

	//TODO: Read the content and add it into the data structure
	vector<vector<float>> data = FileReader::LoadArrayFromFile(file);

	for (vector<float> d : data) {
		tempObject.translate = glm::vec3(d.at(0), d.at(1), d.at(2));
		tempObject.angle = d.at(3);
		tempObject.rotation = glm::vec3(d.at(4), d.at(5), d.at(6));
		tempObject.scale = glm::vec3(d.at(7), d.at(8), d.at(9));
		vGObject.push_back(tempObject);
	}
	return vGObject;
}
