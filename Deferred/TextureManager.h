#ifndef TURRI_TEXTURE_MANAGER
#define TURRI_TEXTURE_MANAGER

#include <string>
#include <vector>
#include <iostream>
#include <tuple>

//Third-party libraries
#include <GL/glew.h>			//The OpenGL Extension Wrangler
#include <glm/glm.hpp>			//OpenGL Mathematics


struct GLTexture {
	GLuint id;
	int width;
	int height;
};

//This class manages all the data related to the different textures used in the game
class TextureManager {
public:
	GLuint getTextureID(std::string filePath);
	GLuint getTextureCubemapID(std::vector<std::string> filePath);
	// Pattern singleton
	static TextureManager& Instance() {
		static TextureManager instance;
		return instance;
	}
	TextureManager(TextureManager const&) = delete;
	void operator=(TextureManager const&) = delete;

	void deleteTextures();

private:
	TextureManager() {};


	GLuint loadTexture(std::string filePath);
	GLuint load3DTexture(std::vector<std::string> filePath);

	std::vector <GLTexture> _textureData;
	std::vector <std::string> _listOfTextures;
};

#endif
