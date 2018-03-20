#ifndef  TURRI_GEOMETRY
#define TURRI_GEOMETRY

#include <vector>
#include "Vertex.h"
#include "FileReader.h"
#include "Entity.h"

class Geometry {
public:

	static  std::vector<GameObject> LoadGameElements(std::string file);

	static OBJ LoadModelFromFile(std::string file);
};

#endif // ! TURRI_GEOMETRY
