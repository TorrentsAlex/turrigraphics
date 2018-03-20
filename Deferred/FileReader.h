#ifndef TURRI_FILEREADER
#define TURRI_FILEREADER

#include <string>
#include <iostream>
#include <fstream>

#include <vector>
#include <sstream>
#include "OBJ.h"

using namespace std;

class FileReader {
private:
	FileReader();
	~FileReader();
	static vector<float> split(const string &line, char delim);
	static vector<string> splitString(const string &line, char delim);

public:

	static vector<vector<float>> LoadArrayFromFile(string fileName);
	static vector<glm::vec3> LoadVec3FromFile(string fileName);

	static OBJ LoadOBJFromFile(string fileName);

	static void SaveArrayToFile(vector<vector<float>> vector, string fileName);

	static bool CheckExistFile(string fileName);

	static string LoadStringFromFile(string fileName);

};
#endif
